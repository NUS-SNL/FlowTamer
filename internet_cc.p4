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
    inout metadata_t meta,
    in ingress_intrinsic_metadata_t ig_intr_md,
    in ingress_intrinsic_metadata_from_parser_t ig_intr_md_from_prsr,
    inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
    inout ingress_intrinsic_metadata_for_tm_t ig_intr_md_for_tm){

	Register<bit<16>, bit<1>>(1) new_rwnd;
    RegisterAction<bit<16>, bit<1>, bit<16>>(new_rwnd)
    get_new_rwnd = { 
        void apply(inout bit<16> register_data, out bit<16> result){
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
	apply {
		if(hdr.ethernet.isValid()){
			if(hdr.ethernet.ether_type == (bit<16>) ether_type_t.ARP){
				// do the broadcast to all involved ports
				ig_intr_md_for_tm.mcast_grp_a = MCAST_GRP_ID;
				ig_intr_md_for_tm.rid = 0;
			}
			else { l2_forward.apply(); }
		}
	
		if(hdr.tcp.isValid()){
			bit<16> new_rwnd;
			new_rwnd = get_new_rwnd.execute(0);
// we want to manipulate the rwnd that receiver is advertising to the sender
			if(new_rwnd!=0 && ig_intr_md_for_tm.ucast_egress_port == 129){
				hdr.tcp.window = new_rwnd;
			}
			// if(ig_intr_md_for_tm.ucast_egress_port == 128){
			// 	hdr.tcp.window = new_rwnd;
			// }
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

	Register<bit<32>, bit<8>>(256) eg_max_deq_qdepth;
	RegisterAction<bit<32>, bit<8>, bit<32>>(eg_max_deq_qdepth)
	store_eg_max_deq_qdepth = { 
		void apply(inout bit<32> register_data){
			if(register_data < (bit<32>)eg_intr_md.deq_qdepth){
				register_data = (bit<32>)eg_intr_md.deq_qdepth;
			}
		}
	};

	apply{
		store_eg_max_deq_qdepth.execute(eg_intr_md.egress_port[7:0]);
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
