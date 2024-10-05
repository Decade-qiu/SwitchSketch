# SwitchSketch

## Introduction

Heavy hitter detection is a fundamental task in network traffic measurement and security. Existing work faces the dilemma of suffering dynamic and imbalanced traffic characteristics or lowering the detection efficiency and flexibility. In this paper, we propose a flexible sketch called SwitchSketch that embraces dynamic and skewed traffic for efficient and accurate heavy-hitter detection. The key idea of SwitchSketch is allowing the sketch to dynamically switch among different modes and take full use of each bit of the memory. We present an encoding-based switching scheme together with a flexible bucket structure to jointly achieve this goal by using a combination of design features, including variable-length cells, counter shrink, embedded metadata, and transferable modes. We further implement SwitchSketch on the NetFPGA-1G-CML board. Experimental results based on real Internet traces show that SwitchSketch achieves a high $\text{F}_\beta$-Score of threshold-𝑡 detection (consistently higher than 0.938) and over 99\% precision rate of top-𝑘 detection under a tight memory size. Besides, it outperforms the state-of-the-art by reducing the ARE by 30.77\%~99.96\%.

## Dataset: 

The datasets we use are two 1-minute traces downloaded from CAIDA-2016(https://catalog.caida.org/details/dataset/passive_2016_pcap) and CAIDA-2019(https://catalog.caida.org/details/dataset/passive_2019_pcap). In the traces, the data items are IP packets, and each flow’s key is identified by the source IP address. The 1-minute CAIDA-2016 trace contains over 31M items belonging to 0.58M flows, and the 1-minute CAIDA-2019 trace contains over 36M items belonging to 0.37M flows.

We have provided a 1-minute CAIDA-2019 dataset in the folder "C++/data" (Due to GitHub's file upload limitations, we provide only a subset of the traffic data from the 1-minute CAIDA-2019 dataset).

## Running

### Environment

- Ubuntu 18.04.4 LTS
- gcc version 7.5.0

### Download repository and run the script 

```bash
git clone https://github.com/Decade-qiu/SwitchSketch.git
cd SwitchSketch
./main.sh
```

## Output Files Overview

After running `main.sh` in the current directory, multiple output files will be generated. Each algorithm's output is provided in pairs of lines:  
- The **first line** contains the algorithm name.  
- The **second line** contains the algorithm's output result.  

### 1. C++/Throughput.txt
This file contains the **insertion throughput** of the algorithm, measured in Mops (Million Operations Per Second).

### 2. Python\offline\result\
This directory contains results for **offline detection**.

- **ARE_memory.txt**: Displays the algorithm's ARE (Average Relative Error) for **threshold-𝑡 detection**. The values from left to right correspond to memory sizes of 50KB, 100KB, 150KB, 200KB, 250KB, and 300KB, respectively.
  
- **PR_k.txt**: Displays the algorithm's **precision rate** for **top-𝑘 detection**. The values from left to right correspond to the k-values of 64, 128, 256, 512, and 1024, respectively.
  
- **PR_memory.txt**: Displays the algorithm's **precision rate** for **top-𝑘 detection**. The values from left to right correspond to memory sizes of 50KB, 100KB, 150KB, 200KB, 250KB, and 300KB, respectively.

### 3. Python\online\result\
This directory contains results for **online detection**.

- **FNR(FPR).txt**: Displays the algorithm's **False Negative Rate (False Positive Rate)**.

- **F2_t.txt**: Displays the algorithm's **$\text{F}_\beta$-Score** for **threshold-𝑡 detection**. The values from left to right correspond to 𝑡-values of 750, 1500, 3000, and 6000, respectively.
  
- **F2_memory.txt**: Displays the algorithm's **$\text{F}_\beta$-Score** for **threshold-𝑡 detection**. The values from left to right correspond to memory sizes of 50KB, 100KB, 150KB, 200KB, 250KB, and 300KB, respectively.
