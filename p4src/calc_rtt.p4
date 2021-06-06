/* -*- P4_16 -*- */
#ifndef _RTT_CALC_P4_
#define _RTT_CALC_P4_

#include <core.p4>
#if __TARGET_TOFINO__ == 2
#include <t2na.p4>
#else
#include <tna.p4>
#endif

#include "includes/headers.p4"
#include "includes/parser.p4"

#define MAX_UINT32 4294967295


#define HASH_TABLE_SIZE 65535
#define HASH_TABLE_OP_SUCCESS 0
#define HASH_TABLE_OP_FAILURE 1

#define TCP_HANDSHAKE_TIMEOUT (50 * 1000 * 1000) // 50ms
#define MAX_LEGIT_RTT     (2000 * 1000 * 1000) // 2000ms


struct hash_table_entry {
    bit<32> fp;  // fingerprint 
    bit<32> ts;  // timestamp
}

// @pa_no_overlay("egress","eg_meta.rtt")
control CalculateRTT(inout header_t hdr, inout egress_metadata_t eg_meta){

    Hash<bit<32>>(HashAlgorithm_t.CRC32) hash_pkt_fingerprint;
    Hash<bit<16>>(HashAlgorithm_t.CRC16) hashfunction_table1;

    bit<32> pkt_fingerprint;
    bit<16> hash_table1_index;
    bit<32> syn_timestamp;
    bit<32> min_val;

    /* NOTE: Expected seq number has already been adjusted for SYN or ACK
       by table 'classify_tcp_pkt' in the egress control. */

    action get_pkt_fingerprint(){
        pkt_fingerprint = hash_pkt_fingerprint.get({
            hdr.ipv4.src_addr, hdr.ipv4.dst_addr,
            hdr.tcp.src_port, hdr.tcp.dst_port,
            eg_meta.expected_seq_no
            });
    }

    action get_htable_index(){
        hash_table1_index = hashfunction_table1.get({
            // 4w0, <-- causing error while loading bfrt python
            hdr.ipv4.src_addr, hdr.ipv4.dst_addr,
            hdr.tcp.src_port, hdr.tcp.dst_port,
            eg_meta.expected_seq_no
            // 4w0
            });
    }

    Register<hash_table_entry, bit<16>>(HASH_TABLE_SIZE, {0, 0}) hash_table1;

    RegisterAction<hash_table_entry, bit<16>, bit<32>>(hash_table1) reg_insert_hash_table1 = {
        void apply(inout hash_table_entry reg_value, out bit<32> rv){

            rv = HASH_TABLE_OP_FAILURE; // default to tell that insertion FAILED
            hash_table_entry orig_val = reg_value;

            bool entry_is_empty = (orig_val.fp == 0);
            bool entry_is_old   = ((eg_meta.curr_time - orig_val.ts) > TCP_HANDSHAKE_TIMEOUT);

            if(entry_is_empty || entry_is_old){ // do the insertion
                reg_value.fp = pkt_fingerprint;
                reg_value.ts = eg_meta.curr_time;
                rv = HASH_TABLE_OP_SUCCESS;
            }

        }
    };

    RegisterAction<hash_table_entry, bit<16>, bit<32>>(hash_table1) reg_lookup_hash_table1 = {
        void apply(inout hash_table_entry reg_value, out bit<32> rv){

            rv = 0; // default to tell that lookup FAILED. BUG?: This HAS to be 0. 32w1 won't work.
            hash_table_entry orig_val = reg_value;

            bool entry_matched = (orig_val.fp == pkt_fingerprint);
            // bool rtt_is_legit  = ((eg_meta.curr_time - orig_val.ts) < MAX_LEGIT_RTT);

            if(entry_matched){ // do the lookup and reset entry | && rtt_is_legit
                reg_value.fp = 0;
                reg_value.ts = 0;
                rv = orig_val.ts;
            }

        }
    };


    action insert_hash_table1(){
        eg_meta.rtt_calc_status = (bit<1>)reg_insert_hash_table1.execute(hash_table1_index);
    }

    action lookup_hash_table1(){
        syn_timestamp = reg_lookup_hash_table1.execute(hash_table1_index);
    }

    Register<bit<32>,bit<1>>(1, 0) reg_adjust_rtt; // size = 1, initial_val = 0
    RegisterAction<bit<32>,bit<1>,bit<32>>(reg_adjust_rtt) do_adjust_rtt = {
        void apply(inout bit<32> reg_val, out bit<32> rv){
            rv = eg_meta.rtt + MAX_UINT32;
        }
    };

    
    action adjust_rtt(){
        eg_meta.rtt = do_adjust_rtt.execute(0);
    }


    apply {
        // compute 'pkt_fingerprint' and 'hash_table1_index'
        get_pkt_fingerprint();
        get_htable_index();

        if(eg_meta.tcp_pkt_type == TCP_PKT_TYPE_SYN){
            insert_hash_table1(); // this will set appropriate eg_meta.rtt_calc_status
            eg_meta.rtt = 0; // no eg_meta.rtt is computed here
        }
        else{
            lookup_hash_table1(); // fills syn_timestamp with SYN ts or ZERO if lookup fails
            if(syn_timestamp != 0){
                // Compute RTT
                eg_meta.rtt = eg_meta.curr_time - syn_timestamp; 
                /* TODO: Try to reduce stages for below wrap-around logic? */
                min_val = min(eg_meta.curr_time, syn_timestamp);
                if(min_val == eg_meta.curr_time){ // eg_meta.curr_time < syn_timestamp 
                    adjust_rtt(); // wrap around correction
                }
                eg_meta.rtt_calc_status = HASH_TABLE_OP_SUCCESS;
            }
            else{
                eg_meta.rtt = 0;
                eg_meta.rtt_calc_status = HASH_TABLE_OP_FAILURE;
            }
        }
    }
}


#endif // header guard

