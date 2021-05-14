/* -*- P4_16 -*- */
#include <core.p4>
#if __TARGET_TOFINO__ == 2
#include <t2na.p4>
#else
#include <tna.p4>
#endif

#include "includes/headers.p4"
#include "includes/parser.p4"

const int MCAST_GRP_ID = 1;


control SwitchIngress(
    inout header_t hdr,
    inout empty_metadata_t meta,
    in ingress_intrinsic_metadata_t ig_intr_md,
    in ingress_intrinsic_metadata_from_parser_t ig_intr_md_from_prsr,
    inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
    inout ingress_intrinsic_metadata_for_tm_t ig_intr_md_for_tm){

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
	apply {
		if(hdr.ethernet.isValid()){
			if(hdr.ethernet.ether_type == (bit<16>) ether_type_t.ARP){
				// do the broadcast to all involved ports
				ig_intr_md_for_tm.mcast_grp_a = MCAST_GRP_ID;
				ig_intr_md_for_tm.rid = 0;
			}
			else { l2_forward.apply(); }
		}
	}

}  // End of SwitchIngressControl


control SwitchEgressControl(
    inout header_t hdr,
    inout metadata_t meta,
    in egress_intrinsic_metadata_t eg_intr_md,
    in egress_intrinsic_metadata_from_parser_t eg_intr_md_from_prsr,
    inout egress_intrinsic_metadata_for_deparser_t eg_intr_md_for_dprsr,
    inout egress_intrinsic_metadata_for_output_port_t eg_intr_md_for_oport){

	bit<16> rwnd;
	bit<1> v;	
	Register<bit<1>, bit<1>>(1) working_copy;
    RegisterAction<bit<1>, bit<1>, bit<1>>(working_copy)
    get_working_copy = { 
        void apply(inout bit<1> register_data, out bit<1> result){
			result = register_data; 
        }
    };
	Register<bit<16>, bit<8>>(256) new_rwnd;
    RegisterAction<bit<16>, bit<8>, bit<16>>(new_rwnd)
    get_new_rwnd = { 
        void apply(inout bit<16> register_data, out bit<16> result){
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

	apply{
		v = get_working_copy.execute(0);
		if(v == 0){
			store_sum_eg_deq_qdepth0.execute(eg_intr_md.egress_port[7:0]);
			store_pkt_count0.execute(eg_intr_md.egress_port[7:0]);
		} else {
			store_sum_eg_deq_qdepth1.execute(eg_intr_md.egress_port[7:0]);
			store_pkt_count1.execute(eg_intr_md.egress_port[7:0]);
		}
		if(hdr.tcp.isValid()){
			rwnd = get_new_rwnd.execute(0);
			if(rwnd!=0 && eg_intr_md.egress_port == 128){ // ack going from receiver to sender
				//hdr.tcp.window = (hdr.tcp.window < rwnd) ? hdr.tcp.window : rwnd;
				hdr.tcp.window = rwnd;
			}
		}

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
