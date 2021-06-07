"""
本脚本用于生成对数/指数查找表
参考论文「Evaluating the Power of Flexible Packet Processing for Network Resource Allocation」
参考代码 https://github.com/sibanez12/perc-p4/blob/aa8721523b171dbbd9d5b77cb42091298f270e04/sw/division/div_impl.py

由 A/B = exp(logA - logB) 可将除法转换为减法，在交换机中配置对数查找表(负责查找logX)以及exp查找表(负责查找exp(X))

对数查找表
    匹配键        掩码    对应值
    N位二进制数    mask   log(Ave) / log(2^N - 1) * (2^l - 1)

    表项: 每个表项由对应的一个 bit string 产生, bit string 的生成见参考论文3.2
    N: 代表数字X的位数
    m: 准确度「窗口大小」(1 ≤ m ≤ N)
    l: 转换精度 log(Ave) => [0, 2^l - 1]
    n: [0,N)「窗口滑动」
    输入值X在表中进行三态匹配(ternary match),即X先和掩码做&运算后再寻找匹配键并获得对应值。
    匹配键 = 00101xxx(bit string) & 11111000(掩码)
    Ave 表示匹配键覆盖范围的平均值
    对应值实际上是将log(Ave)的实际值映射到[0,2^l - 1]这个范围的整数(映射至整数应该是便于交换机存储使用?)

指数查找表
    匹配键               对应值
    Key: [0,2^l - 1]    exp(Key / (2^l - 1) * log(2^N - 1))
"""

from math import log,exp

"""
根据参考论文表2配置参数 N,m,l
"""
N,m,l = 32,6,16
bit_strings = []
log_masks,log_keys,log_aves,log_vals = [],[],[],[]
exp_keys,exp_vals = [],[]

"""
根据参考论文3.2生成 bit string 编码
"""
def gen_bit_strings():
    for n in range(N-1,-1,-1):
        window = min(m-1,N-n-1)
        x_window = max(0,N-n-m)
        for i in range(2**window):
            bit_string = '0'*n + '1' + format(i, '0'+str(window)+'b') + 'x'*x_window
            bit_strings.append(bit_string[:32])
    return bit_strings

"""
根据 bit string 中的忽略位x生成掩码
"""
def gen_masks():
    for s in bit_strings:
        x_num = s.count("x")
        mask = format((2**(N-x_num) - 1)<<x_num, str(N)+"b")
        log_masks.append(mask)
    return log_masks

"""
生成对数表的匹配键
"""
def gen_log_keys():
    for s in bit_strings:
        log_keys.append(s.replace('x','0'))
    return log_keys

"""
计算每个匹配键覆盖范围的平均值
"""
def gen_log_aves():
    for s in bit_strings:
        min_val = int(s.replace('x','0'),2)
        max_val = int(s.replace('x','1'),2)
        log_aves.append((min_val+max_val)/2)
    return log_aves

"""
将log(Ave)映射至[0,2^l - 1]内的整数
"""
def gen_log_vals():
    for ave in log_aves:
        log_vals.append(round(log(ave) / log(2**N - 1) * (2**l - 1)))
    return log_vals


"""
生成[0,2^l - 1]范围内整数的exp查找表
"""
def gen_exp_keys_and_vals():
    for i in range(2**l):
        exp_keys.append(i)
        exp_vals.append(round(exp(i / (2**l - 1) * log(2**N - 1))))
    return exp_keys,exp_vals


if __name__ == "__main__":
    gen_bit_strings()
    gen_masks()
    gen_log_keys()
    gen_log_aves()
    gen_log_vals()
    gen_exp_keys_and_vals()

    print("Num entries for exp table: "+ str(len(exp_keys)))
    print("Num entries for log table: "+ str(len(log_keys)))

    # for i in zip(log_keys,log_masks,log_vals):
    #     print(i[0],int(i[0],base=2),i[1],int(i[1],base=2),i[2])
    # print(len(bit_strings))
    # for i in zip(exp_keys,exp_vals):
    #     print(i[0],i[1])
    # print(len(exp_keys))