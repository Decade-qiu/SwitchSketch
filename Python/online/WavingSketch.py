import mmh3
import collections
import sys

def WavingSketch_Insert(item,index,lst_Counter,lst_Heavy):
    choice = mmh3.hash(item, HASH_SEED_s_choice) % 2
    min_num = sys.maxsize
    min_pos = -1
    heavy_counters = lst_Heavy[index]
    for i in range(d):
        if heavy_counters[i][1] == 0:
            heavy_counters[i][0] = item
            heavy_counters[i][1] = -1
            return
        elif heavy_counters[i][0] == item:
            if heavy_counters[i][1] < 0:
                heavy_counters[i][1] -= 1
            else:
                heavy_counters[i][1] += 1
                lst_Counter[index] += s[choice]
            return
        val = abs(heavy_counters[i][1])
        if val < min_num:
            min_num = val
            min_pos = i

    fi = lst_Counter[index] * s[choice]
    if fi >= min_num:
        if heavy_counters[min_pos][1] < 0:
            min_choice = mmh3.hash(heavy_counters[min_pos][0], HASH_SEED_s_choice) % 2
            lst_Counter[index] -= s[min_choice] * heavy_counters[min_pos][1]

        heavy_counters[min_pos][0] = item
        heavy_counters[min_pos][1] = min_num + 1

    lst_Counter[index] += s[choice]

# Hash function could be defined as needed, in Python, a simple dictionary can be used to store the results
def read_data(file_path):
    # Constants
    KEY_SIZE = 4  # Assuming the KEY_SIZE is 13 bytes (equivalent to C++ uint8_t array)
    TEMP2_SIZE = 9  # Assuming 9 bytes for temp2 in the original code
    # Store keys
    keys = []
    # Store actual flow sizes
    actual_flow_sizes = collections.defaultdict(int)
    
    # Open the binary file
    with open(file_path, 'rb') as fin:
        count = 0
        while True:
            # Read KEY_SIZE bytes
            key = fin.read(KEY_SIZE)
            if len(key) != KEY_SIZE:
                break  # End of file or unexpected size, stop reading
            
            # Append the key
            keys.append(key)
            
            # Update the flow size in the dictionary
            actual_flow_sizes[key] += 1
            count += 1
            
            # Read TEMP2_SIZE bytes (but we don't do anything with it, similar to temp2 in C++)
            temp2 = fin.read(TEMP2_SIZE)
            if len(temp2) != TEMP2_SIZE:
                break  # Break if we can't read the expected number of bytes
            
            # Log progress every 5 million keys
            if count % 5000000 == 0:
                print(f"Successfully read in {file_path}, {count} items")
                # break
        
        print(f"Successfully read in {file_path}, {count} items")
    
    return keys

if __name__ == '__main__':
    # read file
    # df = utils.File_Util.read_single_file(49, True, unique_tag=False)
    # F = df['tag1']
    F = read_data('../../C++/data/20190117-130000-new.dat')
    print('The numbers of packets:{}'.format(len(F)))
    l = collections.Counter(F)

    for _T in [750, 1500, 3000, 6000]:
        upper = _T * 4 / 3
        lst_PR = []
        lst_RR = []
        temp_memo = [50]
        if _T == 750:
            temp_memo = [50, 100, 150, 200, 250, 300]
        for _memorysize in temp_memo:
            memorysize = _memorysize
            alpha = 0.5
            bottom = upper * alpha
            T = (upper + bottom) / 2
            k = 1024
            d = 8
            memorysize += 25
            print('Memory Size:{}KB'.format(memorysize))
            print('T:{}'.format(T))
            print('alpha:{}'.format(alpha))
            print('bottom:{},top:{}'.format(bottom, upper))
            BUCKET_NUM = int(memorysize*1024*8/(52*d+20))

            Counter1 = [0 for bucket in range(BUCKET_NUM)]
            Heavy1 = [[[0,0] for i in range(d)] for bucket in range(BUCKET_NUM)]
            HASH_SEED = [2132,315,1651,3165,4651]

            HASH_SEED_s_choice = HASH_SEED[1]
            HASH_SEED1 = HASH_SEED[0]

            s = [-1,1]
            l1 = dict(l.most_common(k))

            for item in F:
                index1 = mmh3.hash(item, HASH_SEED1) % BUCKET_NUM
                WavingSketch_Insert(item,index1,Counter1,Heavy1)

            # estimated
            result_overT = {}
            result_all = {}
            result_topk = {}

            # real
            real = {}
            real_less_than_bottom = {}
            real_larger_than_upper = {}

            for HeavyPart in Heavy1:
                for cell in HeavyPart:
                    result_all.update({cell[0]:abs(cell[1])})

            for index, item in enumerate(sorted(result_all.items(), key=lambda x: x[1], reverse=True)[:k]):
                result_topk.update({item[0]:item[1]})

            for i in l.items():
                if i[1] > T:
                    real.update({i[0]:i[1]})
                if i[1] < bottom:
                    real_less_than_bottom.update({i[0]:i[1]})
                if i[1] > upper:
                    real_larger_than_upper.update({i[0]:i[1]})
                if i[0] in result_all:
                    if result_all[i[0]] > T:
                        result_overT.update({i[0]:result_all[i[0]]})

            aae_overT = 0
            aae_all = 0
            are_all = 0
            are_overT = 0

            count_overt = 0
            count_bottom = 0
            count_upper_all = len(real_larger_than_upper)

            for key in result_overT.keys():
                if key in real:
                    count_overt += 1
                if l[key] < bottom:
                    count_bottom += 1
                if key in real_larger_than_upper:
                    del  real_larger_than_upper[key]

                aae_overT += abs(result_overT[key] - l[key])
                are_overT += abs(result_overT[key] - l[key]) / l[key]

            count_upper_mis = len(real_larger_than_upper)

            count_topk = 0
            are_topk = 0
            lst = []
            for item in result_topk.items():
                are_topk += abs(l[item[0]] - item[1]) / l[item[0]]
                if item[0] in l1:
                    count_topk += 1
                lst.append([l[item[0]],item[1]])

            FPR = count_bottom / len(real_less_than_bottom)
            FNR = count_upper_mis / count_upper_all
            print('Top-theta are:{}'.format(are_topk / k))
            print('OverT are:{}'.format(are_overT / len(result_overT)))
            print(count_bottom, len(real_less_than_bottom), count_upper_mis, count_upper_all)
            print('FPR:{},FNR:{}'.format(FPR, FNR))
            print(count_overt, len(real), len(result_overT))
            print('Find top-{}:{}'.format(k, count_topk))
            print('Threshold={},PR:{}'.format(T, count_overt / len(result_overT)))
            print('Threshold={},RR:{}'.format(T, count_overt / len(real)))
            lst_PR.append(count_overt / len(result_overT))
            lst_RR.append(count_overt / len(real))
            print('PR', lst_PR)
            print('RR', lst_RR)

            PR = count_overt / len(result_overT)
            RR = count_overt / len(real)

            if _T == 750 and _memorysize == 150:
                with open('./result/FNR.txt', 'a') as f:
                    f.write(f"WavingSketch\n{FNR}\n")
                with open('./result/FPR.txt', 'a') as f:
                    f.write(f"WavingSketch\n{FPR}\n")

            if _T == 750:
                with open('./result/F2_memory.txt', 'a') as f:
                    if _memorysize == 50:
                        f.write(f"WavingSketch\n{5*PR*RR/(4*PR+RR)}")
                    else:
                        f.write(f" {5*PR*RR/(4*PR+RR)}")
                    if _memorysize == 300:
                        f.write('\n')

            if _memorysize == 50:
                with open('./result/F2_t.txt', 'a') as f:
                    if _T == 750:
                        f.write(f"WavingSketch\n{5*PR*RR/(4*PR+RR)}")
                    else:
                        f.write(f" {5*PR*RR/(4*PR+RR)}")
                    if _T == 6000:
                        f.write('\n')