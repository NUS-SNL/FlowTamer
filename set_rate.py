RATE_IN_KBPS = 100000
tm.enable_port_shaping(port=129, dev=0) # rate is in terms of Kbps
tm.set_port_shaping_rate(port=129, pps=False, burstsize=1500, rate=RATE_IN_KBPS, dev=0)   

# Configuring deep buffer in one direction
# Step 1: Reduce buffer allocation to the app pools
# - App pools 1-3 are not being used. So set them to zero
# - App pool 4 is being used by default. Need some small buffer for ACKs to go back
# - App pool 0 is the default pool. It is used when app pool usage is disabled on a queue.
#   Therefore give maximum possible cells to app pool 0.
tm.set_app_pool_size(1, 0)
tm.set_app_pool_size(2, 0)
tm.set_app_pool_size(3, 0)
tm.set_app_pool_size(4, 20)
tm.set_app_pool_size(0, 266240)

# Step 2: Disable app pool usage on the queue requiring deep buffer
tm.disable_q_app_pool_usage(129, 0)
# Step 3: Set the queue's guaranteed min limit
# - This limit somehow cannot be set equal to the app pool 0 size. 
# - In most cases, it goes up to 80% of app pool 0. But sometimes, other limits 
#   play some role. Always test with tm.get_q_usage and UDP traffic to see how 
#   much the queue actually fills up at max. 
tm.set_q_guaranteed_min_limit(129, 0, 196600)  # 15.7 MB


# tm.get_q_usage(1, 129, 0)


