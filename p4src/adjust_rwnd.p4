/* -*- P4_16 -*- */
#ifndef _ADJUST_RWND_P4_
#define _ADJUST_RWND_P4_

#include <core.p4>

#if __TARGET_TOFINO__ == 2
#include <t2na.p4>
#else
#include <tna.p4>
#endif

typedef bit<8> ws_shift_t;
typedef bit<32> rtt_multiplier_t;

#include "includes/headers.p4"
#include "includes/parser.p4"

control adjustRWND(inout header_t hdr, inout ingress_metadata_t ig_meta){
    
    rtt_multiplier_t rtt_multiplier;
    ws_shift_t ws_shift;
    bit<16> log_rwnd;
    bit<16> log_rtt_multiplier;
    bit<32> log_sum; 

    action set_rtt_mul_and_ws(rtt_multiplier_t rtt_mul, ws_shift_t ws){
        rtt_multiplier = rtt_mul;
        ws_shift = ws;
    }

    action miss_rtt_ws_lookup(){
        /* TODO: on miss, inform the CP about it */
    }
    
    table fetch_rtt_mul_and_ws {
        key = {
            hdr.ipv4.src_addr: exact;
            hdr.ipv4.dst_addr: exact;
            hdr.tcp.src_port: exact;
            hdr.tcp.dst_port: exact;
        }
        actions = {
            set_rtt_mul_and_ws;
            miss_rtt_ws_lookup;
        }
        default_action = miss_rtt_ws_lookup;
    }

    action set_log_rwnd(bit<16> result) {
        log_rwnd = result;
    }
    table tbl_log_rwnd {
        key = {
            ig_meta.base_rwnd: ternary;
        }
        actions = {
            set_log_rwnd;
        }

       size = 1024; 
    }

    action set_log_rtt_multiplier(bit<16> result) {
        log_rtt_multiplier = result;
    }
    table tbl_log_rtt_multiplier {
        key = {
            rtt_multiplier: ternary;
        }
        actions = {
            set_log_rtt_multiplier;
        }
       size = 1024;
    }

    action set_rtt_scaled_rwnd(bit<32> result) {
        ig_meta.rtt_scaled_rwnd = result;
    }
    table tbl_antilog_log_sum {
        key = {
            log_sum: exact;
        }
        actions = {
            set_rtt_scaled_rwnd;
        }

        size = 65540;
    }

    action rshift0(){ /* ig_meta.rtt_scaled_rwnd = ig_meta.rtt_scaled_rwnd >> 0;*/ } // nop
    action rshift1(){ ig_meta.rtt_scaled_rwnd = ig_meta.rtt_scaled_rwnd >> 1; }
    action rshift2(){ ig_meta.rtt_scaled_rwnd = ig_meta.rtt_scaled_rwnd >> 2; }
    action rshift3(){ ig_meta.rtt_scaled_rwnd = ig_meta.rtt_scaled_rwnd >> 3; }
    action rshift4(){ ig_meta.rtt_scaled_rwnd = ig_meta.rtt_scaled_rwnd >> 4; }
    action rshift5(){ ig_meta.rtt_scaled_rwnd = ig_meta.rtt_scaled_rwnd >> 5; }
    action rshift6(){ ig_meta.rtt_scaled_rwnd = ig_meta.rtt_scaled_rwnd >> 6; }
    action rshift7(){ ig_meta.rtt_scaled_rwnd = ig_meta.rtt_scaled_rwnd >> 7; }
    action rshift8(){ ig_meta.rtt_scaled_rwnd = ig_meta.rtt_scaled_rwnd >> 8; }
    action rshift9(){ ig_meta.rtt_scaled_rwnd = ig_meta.rtt_scaled_rwnd >> 9; }
    action rshift10(){ ig_meta.rtt_scaled_rwnd = ig_meta.rtt_scaled_rwnd >> 10; }
    action rshift11(){ ig_meta.rtt_scaled_rwnd = ig_meta.rtt_scaled_rwnd >> 11; }
    action rshift12(){ ig_meta.rtt_scaled_rwnd = ig_meta.rtt_scaled_rwnd >> 12; }
    action rshift13(){ ig_meta.rtt_scaled_rwnd = ig_meta.rtt_scaled_rwnd >> 13; }
    action rshift14(){ ig_meta.rtt_scaled_rwnd = ig_meta.rtt_scaled_rwnd >> 14; }
    table tbl_right_shift{
        key = { ws_shift: exact; }
        actions = { rshift0; rshift1; rshift2; rshift3; rshift4; 
                    rshift5; rshift6; rshift7; rshift8; rshift9;
                    rshift10; rshift11; rshift12; rshift13; rshift14; }
        size = 15;
        const entries = {
			0: rshift0();
			1: rshift1();
			2: rshift2();
			3: rshift3();
			4: rshift4();
			5: rshift5();
			6: rshift6();
            7: rshift7();
			8: rshift8();
			9: rshift9();
			10: rshift10();
			11: rshift11();
			12: rshift12();
			13: rshift13();
            14: rshift14();
		}
    }
    apply{
        if(hdr.tcp.isValid()){
            fetch_rtt_mul_and_ws.apply();
            
            tbl_log_rtt_multiplier.apply();
            tbl_log_rwnd.apply();
            
            log_sum = (bit<32>)(log_rwnd + log_rtt_multiplier);
            
            tbl_antilog_log_sum.apply();
            tbl_right_shift.apply(); 
        }
    }
}
#endif