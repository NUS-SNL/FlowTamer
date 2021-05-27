#ifndef _HEADERS_
#define _HEADERS_

typedef bit<48> mac_addr_t;
typedef bit<32> ipv4_addr_t;

typedef bit<4> tcp_pkt_type_t;
const tcp_pkt_type_t TCP_PKT_TYPE_SYN     = 1;
const tcp_pkt_type_t TCP_PKT_TYPE_SYN_ACK = 2;
const tcp_pkt_type_t TCP_PKT_TYPE_FIN     = 3;
const tcp_pkt_type_t TCP_PKT_TYPE_FIN_ACK = 4;
const tcp_pkt_type_t TCP_PKT_TYPE_ACK     = 5;
const tcp_pkt_type_t TCP_PKT_TYPE_DATA    = 6;

header ethernet_h {
	mac_addr_t dst_addr;
	mac_addr_t src_addr;
	bit<16> ether_type;
}

header arp_h {
    bit<16> htype;
    bit<16> ptype;
    bit<8> hlen;
    bit<8> plen;
    bit<16> oper;
    mac_addr_t sender_hw_addr;
    ipv4_addr_t sender_ip_addr;
    mac_addr_t target_hw_addr;
    ipv4_addr_t target_ip_addr;
}

header ipv4_h {
	bit<4> version;
    bit<4> ihl;
    bit<8> diffserv;
    bit<16> total_len;
    bit<16> identification;
    bit<3> flags;
    bit<13> frag_offset;
    bit<8> ttl;
    bit<8> protocol;
    bit<16> hdr_checksum;
    ipv4_addr_t src_addr;
    ipv4_addr_t dst_addr;
}

header tcp_h {
    bit<16> src_port;
    bit<16> dst_port;
    bit<32> seq_no;
    bit<32> ack_no;
    bit<4> data_offset;
    bit<4> res;
    bit<8> flags;
 /* bit<1> cwr;
    bit<1> ece;
    bit<1> urg;
    bit<1> ack;
    bit<1> psh;
    bit<1> rst;
    bit<1> syn;
    bit<1> fin; */
    bit<16> window;
    bit<16> checksum;
    bit<16> urgent_ptr;
}

header udp_h {
    bit<16> src_port;
    bit<16> dst_port;
    bit<16> hdr_length;
    bit<16> checksum;
}

// internal hdr to indicate normal, bridged, 
// ig_mirrored, eg_mirrored pkts to egress parser

typedef bit<4> internal_hdr_type_t;
typedef bit<4> internal_hdr_info_t;

const internal_hdr_type_t INTERNAL_HDR_TYPE_BRIDGED_META = 0xA;
const internal_hdr_type_t INTERNAL_HDR_TYPE_IG_MIRROR = 0xB;
const internal_hdr_type_t INTERNAL_HDR_TYPE_EG_MIRROR = 0xC;

/* Mirror Types */
const bit<3> EG_MIRROR1 = 1; // corresponds to eg_mirror1_h


#define INTERNAL_HEADER           \
    internal_hdr_type_t type; \
    internal_hdr_info_t info


header internal_hdr_h {
    INTERNAL_HEADER;
}

header bridged_meta_h {
    INTERNAL_HEADER;
    /* Add any metadata to be bridged from ig to eg */
}

header eg_mirror1_h {
    INTERNAL_HEADER;
    bit<48> eg_global_ts;
}

struct header_t {
    bridged_meta_h bridged_meta;
	ethernet_h ethernet;
	ipv4_h ipv4;
    arp_h arp;
	tcp_h tcp;
	udp_h udp;
}

struct ingress_metadata_t {
    bit<16> l4_payload_checksum;
}

struct egress_metadata_t {
    bridged_meta_h bridged;
    eg_mirror1_h eg_mirror1;
    MirrorId_t mirror_session;
    internal_hdr_type_t internal_hdr_type;
    internal_hdr_info_t internal_hdr_info;
    tcp_pkt_type_t tcp_pkt_type;
    bit<1> rtt_calc_status;
    bit<32> expected_seq_no;
    bit<32> rtt;
}


#endif
