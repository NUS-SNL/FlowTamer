#ifndef _PARSER_
#define _PARSER_

#define IP_PROTOCOL_UDP 17
#define IP_PROTOCOL_TCP 6


enum bit<16> ether_type_t {
    IPV4 = 0x0800,
    ARP  = 0x0806
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
    out metadata_t meta,
    out ingress_intrinsic_metadata_t ig_intr_md,
    out ingress_intrinsic_metadata_for_tm_t ig_intr_md_for_tm,
    out ingress_intrinsic_metadata_from_parser_t ig_intr_md_from_prsr){

    Checksum() tcp_checksum;
	state start {
        pkt.extract(ig_intr_md);
        pkt.advance(PORT_METADATA_SIZE); // macro defined in tofino.p4
		transition parse_ethernet;
	}

	state parse_ethernet {
		pkt.extract(hdr.ethernet);
		transition select(hdr.ethernet.ether_type){
			(bit<16>) ether_type_t.IPV4: parse_ipv4;
			(bit<16>) ether_type_t.ARP: parse_arp;
			default: accept;
		}
	}

	state parse_ipv4 {
		pkt.extract(hdr.ipv4);
        tcp_checksum.subtract({
            hdr.ipv4.src_addr,
            hdr.ipv4.dst_addr,
            8w0, hdr.ipv4.protocol });
		transition select(hdr.ipv4.protocol){
			(bit<8>) ipv4_proto_t.TCP: parse_tcp;
			(bit<8>) ipv4_proto_t.UDP: parse_udp;
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
            hdr.tcp.data_offset, hdr.tcp.res, hdr.tcp.flags,
            hdr.tcp.window,
            hdr.tcp.checksum,
            hdr.tcp.urgent_ptr });
        meta.l4_payload_checksum = tcp_checksum.get();
		transition accept;
	}

	state parse_udp {
		pkt.extract(hdr.udp);
		transition accept;
	}
}


// ---------------------------------------------------------------------------
// Ingress Deparser
// ---------------------------------------------------------------------------
control SwitchIngressDeparser(
        packet_out pkt,
        inout header_t hdr,
        in metadata_t meta,
        in ingress_intrinsic_metadata_for_deparser_t ig_dprsr_md) {

    Checksum() ipv4_checksum;
    Checksum() tcp_checksum;
    apply {
        hdr.ipv4.hdr_checksum = ipv4_checksum.update({
            hdr.ipv4.version,
            hdr.ipv4.ihl,
            hdr.ipv4.diffserv,
            hdr.ipv4.total_len,
            hdr.ipv4.identification,
            hdr.ipv4.flags,
            hdr.ipv4.frag_offset,
            hdr.ipv4.ttl,
            hdr.ipv4.protocol,
            hdr.ipv4.src_addr,
            hdr.ipv4.dst_addr});

        hdr.tcp.checksum = tcp_checksum.update({
            hdr.ipv4.src_addr,
            hdr.ipv4.dst_addr,
            8w0, hdr.ipv4.protocol,
            hdr.tcp.src_port,
            hdr.tcp.dst_port,
            hdr.tcp.seq_no,
            hdr.tcp.ack_no,
            hdr.tcp.data_offset, hdr.tcp.res, hdr.tcp.flags,
            hdr.tcp.window,
            hdr.tcp.urgent_ptr,
            meta.l4_payload_checksum });
        pkt.emit(hdr);
    }
}


// ---------------------------------------------------------------------------
// Egress parser
// ---------------------------------------------------------------------------
parser SwitchEgressParser(
    packet_in pkt,
    out header_t hdr,
    out metadata_t meta,
    out egress_intrinsic_metadata_t eg_intr_md,
    out egress_intrinsic_metadata_from_parser_t eg_intr_md_from_prsr){

    state start {
        pkt.extract(eg_intr_md);
        transition parse_ethernet;
    }

    state parse_ethernet {
    	pkt.extract(hdr.ethernet);
    	transition accept;
    }

    // do more stuff here if needed
}

// ---------------------------------------------------------------------------
// Egress Deparser
// ---------------------------------------------------------------------------
control SwitchEgressDeparser(
    packet_out pkt,
    inout header_t hdr,
    in metadata_t meta,
    in egress_intrinsic_metadata_for_deparser_t eg_intr_md_for_dprsr,
    in egress_intrinsic_metadata_t eg_intr_md,
    in egress_intrinsic_metadata_from_parser_t eg_intr_md_from_prsr){

	apply{
        // do more stuff here if needed

		pkt.emit(hdr);
	}

}







#endif