#ifndef _PARSER_
#define _PARSER_

#define IP_PROTOCOL_UDP 17
#define IP_PROTOCOL_TCP 6


enum bit<16> ether_type_t {
    IPV4 = 0x0800,
    ARP  = 0x0806,
    PKTGEN = 0xBFBF
}

enum bit<8> ipv4_proto_t {
    TCP = 6,
    UDP = 17
}


// ---------------------------------------------------------------------------
// Ingress parser
// ---------------------------------------------------------------------------
parser SwitchIngressParser(
    packet_in pkt,
    out header_t hdr,
    out ingress_metadata_t ig_meta,
    out ingress_intrinsic_metadata_t ig_intr_md,
    out ingress_intrinsic_metadata_for_tm_t ig_intr_md_for_tm,
    out ingress_intrinsic_metadata_from_parser_t ig_intr_md_from_prsr){

    Checksum() tcp_checksum;
	state start {
        pkt.extract(ig_intr_md);
        pkt.advance(PORT_METADATA_SIZE); // macro defined in tofino.p4
        transition init_metadata;
	}

    state init_metadata { // init bridged_meta (based on slide 23 of BA-1122)
        ig_meta.l4_payload_checksum = 0;
        ig_meta.base_rwnd = 0;
        ig_meta.rtt_scaled_rwnd = 0;
        // ig_meta.port_meta.apply_algo = 0;
        hdr.bridged_meta.setValid();
        hdr.bridged_meta.type = INTERNAL_HDR_TYPE_BRIDGED_META;
        hdr.bridged_meta.info = 0;
        // transition parse_port_metadata;
        transition parse_ethernet;
    }
/* 
    state parse_port_metadata {
        ig_meta.port_meta = port_metadata_unpack<port_metadata_t>(pkt);
        transition parse_ethernet;
    }
 */

	state parse_ethernet {
		pkt.extract(hdr.ethernet);
		transition select(hdr.ethernet.ether_type){
			(bit<16>) ether_type_t.IPV4: parse_ipv4;
			(bit<16>) ether_type_t.ARP: parse_arp;
            (bit<16>) ether_type_t.PKTGEN : parse_algorwnd_update;
			default: accept;
		}
	}

    state parse_algorwnd_update {
        pkt.extract(hdr.algorwnd_update);
        transition accept;
    }

	state parse_ipv4 {
		pkt.extract(hdr.ipv4);

        tcp_checksum.subtract({
            hdr.ipv4.src_addr,
            hdr.ipv4.dst_addr,
            8w0, hdr.ipv4.protocol });
		
        transition select(hdr.ipv4.protocol, hdr.ipv4.total_len){	
            ((bit<8>) ipv4_proto_t.TCP, 80..65535): parse_tcp_data;
            ((bit<8>) ipv4_proto_t.TCP, _): parse_tcp;
		    default: accept;
		}
	}

	state parse_arp {
		pkt.extract(hdr.arp);
		transition accept;
	}

	state parse_tcp {
		pkt.extract(hdr.tcp);
        tcp_checksum.subtract({
            hdr.tcp.src_port,
            hdr.tcp.dst_port,
            hdr.tcp.seq_no,
            hdr.tcp.ack_no,
            hdr.tcp.data_offset, hdr.tcp.res, 
            hdr.tcp.flags,
            hdr.tcp.window,
            hdr.tcp.checksum,
            hdr.tcp.urgent_ptr });
        ig_meta.l4_payload_checksum = tcp_checksum.get();
        ig_meta.csum_update_type = 0;
		transition accept;
	}

    state parse_tcp_data {
		pkt.extract(hdr.tcp);
        pkt.extract(hdr.innetworkcc_info);

        tcp_checksum.subtract({
            hdr.tcp.src_port,
            hdr.tcp.dst_port,
            hdr.tcp.seq_no,
            hdr.tcp.ack_no,
            hdr.tcp.data_offset, 
            hdr.tcp.res,
            hdr.tcp.flags,
            hdr.tcp.window,
            hdr.tcp.checksum,
            hdr.tcp.urgent_ptr,
            hdr.innetworkcc_info.algo_rwnd,
            hdr.innetworkcc_info.rtt_mul,
            hdr.innetworkcc_info.qdepth_sum,
            hdr.innetworkcc_info.pkt_count,
            hdr.innetworkcc_info.qdepth, 
            hdr.innetworkcc_info.final_rwnd,
            hdr.innetworkcc_info.ws
            });

        ig_meta.l4_payload_checksum = tcp_checksum.get();
        ig_meta.csum_update_type = 1;

		transition accept;
    }
}


// ---------------------------------------------------------------------------
// Ingress Deparser
// ---------------------------------------------------------------------------
control SwitchIngressDeparser(
        packet_out pkt,
        inout header_t hdr,
        in ingress_metadata_t ig_meta,
        in ingress_intrinsic_metadata_for_deparser_t ig_dprsr_md) {

    Checksum() tcp_checksum;    
    apply {

        // if(hdr.innetworkcc_info.isValid()){
        if(ig_meta.csum_update_type == 1){
            hdr.tcp.checksum = tcp_checksum.update({
                hdr.ipv4.src_addr,
                hdr.ipv4.dst_addr,
                8w0, hdr.ipv4.protocol,
                hdr.tcp.src_port,
                hdr.tcp.dst_port,
                hdr.tcp.seq_no,
                hdr.tcp.ack_no,
                hdr.tcp.data_offset, 
                hdr.tcp.res, 
                hdr.tcp.flags,
                hdr.tcp.window,
                hdr.tcp.urgent_ptr,
                hdr.innetworkcc_info.algo_rwnd,
                hdr.innetworkcc_info.rtt_mul,
                hdr.innetworkcc_info.qdepth_sum,
                hdr.innetworkcc_info.pkt_count,
                hdr.innetworkcc_info.qdepth, 
                hdr.innetworkcc_info.final_rwnd,
                hdr.innetworkcc_info.ws,
                ig_meta.l4_payload_checksum
            }); 
        }
        else {
            hdr.tcp.checksum = tcp_checksum.update({
                hdr.ipv4.src_addr,
                hdr.ipv4.dst_addr,
                8w0, hdr.ipv4.protocol,
                hdr.tcp.src_port,
                hdr.tcp.dst_port,
                hdr.tcp.seq_no,
                hdr.tcp.ack_no,
                hdr.tcp.data_offset, hdr.tcp.res, 
                hdr.tcp.flags,
            /*  hdr.tcp.cwr,
                hdr.tcp.ece,
                hdr.tcp.urg,
                hdr.tcp.ack,
                hdr.tcp.psh,
                hdr.tcp.rst,
                hdr.tcp.syn,
                hdr.tcp.fin, */
                hdr.tcp.window,
                hdr.tcp.urgent_ptr,
                ig_meta.l4_payload_checksum }); 
        }

        pkt.emit(hdr);
    }
}


// ---------------------------------------------------------------------------
// Egress parser
// ---------------------------------------------------------------------------
parser SwitchEgressParser(
    packet_in pkt,
    out header_t hdr,
    out egress_metadata_t eg_meta,
    out egress_intrinsic_metadata_t eg_intr_md,
    out egress_intrinsic_metadata_from_parser_t eg_intr_md_from_prsr){

    Checksum() tcp_checksum;
    internal_hdr_h internal_hdr;
    state start {
        pkt.extract(eg_intr_md);
        
        internal_hdr = pkt.lookahead<internal_hdr_h>();
        transition select(internal_hdr.type, internal_hdr.info){
            (INTERNAL_HDR_TYPE_BRIDGED_META, _): parse_bridged_meta;
            (INTERNAL_HDR_TYPE_EG_MIRROR, (bit<4>)EG_MIRROR_TYPE_1): parse_eg_mirror1;
        }
    }

    state parse_bridged_meta {
        pkt.extract(eg_meta.bridged);
        transition parse_ethernet;
    }

    state parse_eg_mirror1 {
        pkt.extract(eg_meta.eg_mirror1);
        transition parse_only_ethernet;
    }

    state parse_only_ethernet {
		pkt.extract(hdr.ethernet);
        transition accept;
    }

    state parse_ethernet {
		pkt.extract(hdr.ethernet);
		transition select(hdr.ethernet.ether_type){
			(bit<16>) ether_type_t.IPV4: parse_ipv4;
			(bit<16>) ether_type_t.ARP: parse_arp;
            (bit<16>) ether_type_t.PKTGEN : parse_algorwnd_update;
			default: accept;
		}
	}

    state parse_algorwnd_update {
        pkt.extract(hdr.algorwnd_update);
        transition accept;
    }

	state parse_ipv4 {
		pkt.extract(hdr.ipv4);

        tcp_checksum.subtract({
            hdr.ipv4.src_addr,
            hdr.ipv4.dst_addr,
            8w0, hdr.ipv4.protocol });
		
		transition select(hdr.ipv4.protocol, hdr.ipv4.total_len){	
            ((bit<8>) ipv4_proto_t.TCP, 80..65535): parse_tcp_data;
            ((bit<8>) ipv4_proto_t.TCP, _): parse_tcp;
		    default: accept;
		}
	}

	state parse_arp {
		pkt.extract(hdr.arp);
		transition accept;
	}

	state parse_tcp {
		pkt.extract(hdr.tcp);
		tcp_checksum.subtract({
            hdr.tcp.src_port,
            hdr.tcp.dst_port,
            hdr.tcp.seq_no,
            hdr.tcp.ack_no,
            hdr.tcp.data_offset, hdr.tcp.res, 
            hdr.tcp.flags,
            hdr.tcp.window,
            hdr.tcp.checksum,
            hdr.tcp.urgent_ptr });
        eg_meta.l4_payload_checksum = tcp_checksum.get();
        
        transition accept;
	}

    state parse_tcp_data {
		pkt.extract(hdr.tcp);
        pkt.extract(hdr.innetworkcc_info);

        tcp_checksum.subtract({
            hdr.tcp.src_port,
            hdr.tcp.dst_port,
            hdr.tcp.seq_no,
            hdr.tcp.ack_no,
            hdr.tcp.data_offset, 
            hdr.tcp.res,
            hdr.tcp.flags,
            hdr.tcp.window,
            hdr.tcp.checksum,
            hdr.tcp.urgent_ptr,
            hdr.innetworkcc_info.algo_rwnd,
            hdr.innetworkcc_info.rtt_mul,
            hdr.innetworkcc_info.qdepth_sum,
            hdr.innetworkcc_info.pkt_count, 
            hdr.innetworkcc_info.qdepth, 
            hdr.innetworkcc_info.final_rwnd,
            hdr.innetworkcc_info.ws
            });

        eg_meta.l4_payload_checksum = tcp_checksum.get();
		transition accept;
    }    
}

// ---------------------------------------------------------------------------
// Egress Deparser
// ---------------------------------------------------------------------------
control SwitchEgressDeparser(
    packet_out pkt,
    inout header_t hdr,
    in egress_metadata_t eg_meta,
    in egress_intrinsic_metadata_for_deparser_t eg_intr_md_for_dprsr,
    in egress_intrinsic_metadata_t eg_intr_md,
    in egress_intrinsic_metadata_from_parser_t eg_intr_md_from_prsr){

    Checksum() tcp_checksum;
    Mirror() mirror;

    apply {
        
        if(eg_intr_md_for_dprsr.mirror_type == EG_MIRROR_TYPE_1){
            mirror.emit<eg_mirror1_h>(
                eg_meta.mirror_session,
                {
                    eg_meta.internal_hdr_type,
                    eg_meta.internal_hdr_info,
                    eg_meta.ts_to_report1,
                    eg_meta.ts_to_report2
                }
                );
        } // end of if
        if(hdr.innetworkcc_info.isValid()){
            hdr.tcp.checksum = tcp_checksum.update({
                hdr.ipv4.src_addr,
                hdr.ipv4.dst_addr,
                8w0, hdr.ipv4.protocol,
                hdr.tcp.src_port,
                hdr.tcp.dst_port,
                hdr.tcp.seq_no,
                hdr.tcp.ack_no,
                hdr.tcp.data_offset, 
                hdr.tcp.res,
                hdr.tcp.flags,
                hdr.tcp.window,
                hdr.tcp.urgent_ptr,
                hdr.innetworkcc_info.algo_rwnd,
                hdr.innetworkcc_info.rtt_mul,
                hdr.innetworkcc_info.qdepth_sum,
                hdr.innetworkcc_info.pkt_count,
                hdr.innetworkcc_info.qdepth, 
                hdr.innetworkcc_info.final_rwnd,
                hdr.innetworkcc_info.ws,
                eg_meta.l4_payload_checksum
            }); 
        }
        else{
            hdr.tcp.checksum = tcp_checksum.update({
                hdr.ipv4.src_addr,
                hdr.ipv4.dst_addr,
                8w0, hdr.ipv4.protocol,
                hdr.tcp.src_port,
                hdr.tcp.dst_port,
                hdr.tcp.seq_no,
                hdr.tcp.ack_no,
                hdr.tcp.data_offset, hdr.tcp.res, 
                hdr.tcp.flags,
                hdr.tcp.window,
                hdr.tcp.urgent_ptr,
                eg_meta.l4_payload_checksum }); 
        }

        pkt.emit(hdr);

    }
}

#endif