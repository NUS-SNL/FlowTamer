/* -*- P4_16 -*- */
#include <core.p4>
#if __TARGET_TOFINO__ == 2
#include <t2na.p4>
#else
#include <tna.p4>
#endif

#include "includes/headers.p4"
#include "includes/parser.p4"
#include "rtt_calc.p4"
#include "adjust_rwnd.p4"

#define MIRROR_SESSION_ENP4S0F0 1

const int MCAST_GRP_ID = 1;

const PortId_t CPU_ETHERNET_PORT_1 = 64;
const PortId_t CPU_ETHERNET_PORT_2 = 66;


const bit<8> TCP_FLAG_FIN = 1;
const bit<8> TCP_FLAG_SYN = 2;
const bit<8> TCP_FLAG_RST = 4;
const bit<8> TCP_FLAG_PSH = 8;
const bit<8> TCP_FLAG_ACK = 16;

control SwitchIngress(
    inout header_t hdr,
    inout ingress_metadata_t ig_meta,
    in ingress_intrinsic_metadata_t ig_intr_md,
    in ingress_intrinsic_metadata_from_parser_t ig_intr_md_from_prsr,
    inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
    inout ingress_intrinsic_metadata_for_tm_t ig_intr_md_for_tm){

	Register<bit<32>, bit<8>>(256) new_rwnd;
    RegisterAction<bit<32>, bit<8>, bit<32>>(new_rwnd)
    get_new_rwnd = { 
        void apply(inout bit<32> register_data, out bit<32> result){
			result = register_data; 
        }
    };
	action miss(bit<3> drop_bits) {
		ig_intr_md_for_dprsr.drop_ctl = drop_bits;
	}

	action forward(PortId_t port){
		ig_intr_md_for_tm.ucast_egress_port = port;
	}
	table l2_forward {
		key = {
			hdr.ethernet.dst_addr: exact;
		}
		actions = {
			forward;
			@defaultonly miss;
		}
		const default_action = miss(0x1);
	}

	adjustRWND() adjust_rwnd;

	action read_new_rwnd_from_reg(){
		ig_meta.base_rwnd = get_new_rwnd.execute(ig_intr_md.ingress_port[7:0]);
	}
	
	apply {
		if(hdr.ethernet.isValid()){
			if(hdr.ethernet.ether_type == (bit<16>) ether_type_t.ARP){
				// do the broadcast to all involved ports
				ig_intr_md_for_tm.mcast_grp_a = MCAST_GRP_ID;
				ig_intr_md_for_tm.rid = 0;
			}
			else { l2_forward.apply(); }
		}
		if(hdr.tcp.isValid() && (hdr.tcp.flags & 0b00000010) !=1){ // && ig_meta.port_meta.apply_algo ==
			// TCP pkts except SYN and SYN-ACK
			
			read_new_rwnd_from_reg(); // returns the value from register in ig_meta.base_rwnd
			
			if(ig_meta.base_rwnd != 0){ 
				// if we see zero rwnd then we don't do any rwnd adjustment + setting in the packet

				adjust_rwnd.apply(hdr, ig_meta);

				// ASSUMPTION: by this point 'rtt_scaled_rwnd' variable would be WS adjusted and no more than 65535. BUG possibility for high new_rwnd/rtt and low WS
				hdr.tcp.window = min(hdr.tcp.window, ig_meta.rtt_scaled_rwnd[15:0]);
				
			}		
		}
	}

}  // End of SwitchIngressControl

// @pa_no_overlay("egress","eg_meta.rtt")
control SwitchEgressControl(
    inout header_t hdr,
    inout egress_metadata_t eg_meta,
    in egress_intrinsic_metadata_t eg_intr_md,
    in egress_intrinsic_metadata_from_parser_t eg_intr_md_from_prsr,
    inout egress_intrinsic_metadata_for_deparser_t eg_intr_md_for_dprsr,
    inout egress_intrinsic_metadata_for_output_port_t eg_intr_md_for_oport){

	action _nop(){

	}

	bit<1> v;	
	Register<bit<1>, bit<1>>(1) working_copy;
    RegisterAction<bit<1>, bit<1>, bit<1>>(working_copy)
    get_working_copy = { 
        void apply(inout bit<1> register_data, out bit<1> result){
			result = register_data; 
        }
    };
	Register<bit<32>, bit<8>>(256) sum_eg_deq_qdepth0;
	RegisterAction<bit<32>, bit<8>, bit<32>>(sum_eg_deq_qdepth0)
	store_sum_eg_deq_qdepth0 = { 
		void apply(inout bit<32> register_data){
			register_data = register_data + (bit<32>)eg_intr_md.deq_qdepth;
		}
	};
	Register<bit<32>, bit<8>>(256) pkt_count0;
	RegisterAction<bit<32>, bit<8>, bit<32>>(pkt_count0)
	store_pkt_count0 = { 
		void apply(inout bit<32> register_data){
			register_data = register_data + 1;
		}
	};
	Register<bit<32>, bit<8>>(256) sum_eg_deq_qdepth1;
	RegisterAction<bit<32>, bit<8>, bit<32>>(sum_eg_deq_qdepth1)
	store_sum_eg_deq_qdepth1 = { 
		void apply(inout bit<32> register_data){
			register_data = register_data + (bit<32>)eg_intr_md.deq_qdepth;
		}
	};
	Register<bit<32>, bit<8>>(256) pkt_count1;
	RegisterAction<bit<32>, bit<8>, bit<32>>(pkt_count1)
	store_pkt_count1 = { 
		void apply(inout bit<32> register_data){
			register_data = register_data + 1;
		}
	};


	action set_tcp_pkt_type(tcp_pkt_type_t pkt_type){
		eg_meta.tcp_pkt_type = pkt_type;
	}
	action set_SYN_pkt_type_and_expected_seq(){
		eg_meta.tcp_pkt_type = TCP_PKT_TYPE_SYN; 
		eg_meta.expected_seq_no = hdr.tcp.seq_no + 1;
		eg_meta.curr_time = eg_intr_md_from_prsr.global_tstamp[31:0];
	}
	action set_SYN_pkt_type_and_expected_seq_adjust_ts(){ 
		eg_meta.tcp_pkt_type = TCP_PKT_TYPE_SYN; 
		eg_meta.expected_seq_no = hdr.tcp.seq_no + 1;
		// Ensures no ZERO ts inserted in hashtable
		eg_meta.curr_time = eg_intr_md_from_prsr.global_tstamp[31:0] + 1;
	}
	action set_ACK_pkt_type_and_expected_seq(){
		eg_meta.tcp_pkt_type = TCP_PKT_TYPE_ACK; 
		eg_meta.expected_seq_no = hdr.tcp.seq_no;
		eg_meta.curr_time = eg_intr_md_from_prsr.global_tstamp[31:0];
	}


	table prepare_tcp_meta_info{
		key = {
			hdr.tcp.flags: ternary;
			eg_intr_md_from_prsr.global_tstamp[31:0]: ternary;
			hdr.ipv4.total_len: range;
		}
		actions = {
			set_tcp_pkt_type;
			set_SYN_pkt_type_and_expected_seq;
			set_SYN_pkt_type_and_expected_seq_adjust_ts;
			set_ACK_pkt_type_and_expected_seq;
			_nop;
		}
		default_action = _nop;
		size = 16;
		const entries = {
			(TCP_FLAG_SYN, 0, _): set_SYN_pkt_type_and_expected_seq_adjust_ts();
			(TCP_FLAG_SYN, _, _): set_SYN_pkt_type_and_expected_seq();
			(TCP_FLAG_SYN + TCP_FLAG_ACK, _, _): set_tcp_pkt_type(TCP_PKT_TYPE_SYN_ACK);
			(TCP_FLAG_FIN, _, _): set_tcp_pkt_type(TCP_PKT_TYPE_FIN);
			(TCP_FLAG_FIN + TCP_FLAG_ACK, _, _): set_tcp_pkt_type(TCP_PKT_TYPE_FIN_ACK);
			(TCP_FLAG_ACK, _, 0..80): set_ACK_pkt_type_and_expected_seq();
			(_, _, 81..1600): set_tcp_pkt_type(TCP_PKT_TYPE_DATA);
		}
	}


	action mirror_to_report_ts(){
		eg_intr_md_for_dprsr.mirror_type = EG_MIRROR_TYPE_1;
		eg_meta.mirror_session = MIRROR_SESSION_ENP4S0F0;
		eg_meta.internal_hdr_type = INTERNAL_HDR_TYPE_EG_MIRROR;
    	eg_meta.internal_hdr_info = (bit<4>)EG_MIRROR_TYPE_1;
	}

	CalculateRTT() calculate_rtt; // instantiate the control


	apply{
		
		if(eg_meta.eg_mirror1.isValid()){ // mirrored pkt
			// Copy the eg_global_ts as src Eth address
			 hdr.ethernet.src_addr = eg_meta.eg_mirror1.timestamp;
			 
		} // end of mirrored pkt processing
		else { // normal pkt

			if(hdr.tcp.isValid()){
				// set eg_meta: expected_seq_no, curr_time, tcp_pkt_type
				prepare_tcp_meta_info.apply(); 
			}

			v = get_working_copy.execute(0);
			if(v == 0){
				store_sum_eg_deq_qdepth0.execute(eg_intr_md.egress_port[7:0]);
				store_pkt_count0.execute(eg_intr_md.egress_port[7:0]);
			} else {
				store_sum_eg_deq_qdepth1.execute(eg_intr_md.egress_port[7:0]);
				store_pkt_count1.execute(eg_intr_md.egress_port[7:0]);
			}



			if(eg_meta.tcp_pkt_type == TCP_PKT_TYPE_SYN || eg_meta.tcp_pkt_type == TCP_PKT_TYPE_SYN_ACK){ // either SYN or SYN-ACK
				/* Set timestamp to report to CP and mirror the packet */
				eg_meta.ts_to_report = eg_intr_md_from_prsr.global_tstamp;
				mirror_to_report_ts();
			}

			/* If it is SYN or ACK, do the RTT calculation */
			if(eg_meta.tcp_pkt_type == TCP_PKT_TYPE_SYN || eg_meta.tcp_pkt_type == TCP_PKT_TYPE_ACK){
				calculate_rtt.apply(hdr, eg_meta);

				if(eg_meta.tcp_pkt_type == TCP_PKT_TYPE_ACK && eg_meta.rtt_calc_status == HASH_TABLE_OP_SUCCESS){
					// Mirror the ACK packet and include RTT in it
					eg_meta.ts_to_report = (bit<48>)eg_meta.rtt;
					mirror_to_report_ts();
				}
			}

		} // end of normal pkt processing

		/* Making sure to remove the bridged_meta before sending on wire */
		hdr.bridged_meta.setInvalid(); 

		
	}

} // End of SwitchEgressControl


Pipeline(SwitchIngressParser(),
		 SwitchIngress(),
		 SwitchIngressDeparser(),
		 SwitchEgressParser(),
		 SwitchEgressControl(),
		 SwitchEgressDeparser()
		 ) pipe;

Switch(pipe) main;
