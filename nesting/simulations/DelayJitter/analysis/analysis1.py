import sys
import matplotlib.pyplot as plt
import numpy as np
import random

# 打开文件
with open('../Demo_04/results/Dst.txt', 'r') as file:
    lines = file.readlines()

# 读取数据
delay_table = []
delay_table_1 = []
delay_table_3 = []
delay_table_4 = []
delay_table_5 = []
delay_table_7 = []
delay_table_8 = []
time_1 = []
time_3 = []
time_4 = []
time_5 = []
time_7 = []
time_8 = []
for line in lines:
    line = line.strip('\n')
    dict = eval(line)
    delay_table.append(dict)

# 读取不同流延迟
for item in delay_table:
    if item['src'] == "00-00-00-00-00-01":
        time_1.append(item['time'] * 1000) # ms
        delay_table_1.append(item['e2edelay'] * 1000000) # us
    elif item['src'] == "00-00-00-00-00-03":
        time_3.append(item['time'] * 1000) # ms
        delay_table_3.append(item['e2edelay'] * 1000000) # us
    elif item['src'] == "00-00-00-00-00-04":
        time_4.append(item['time'] * 1000) # ms
        delay_table_4.append(item['e2edelay'] * 1000000) # us
    elif item['src'] == "00-00-00-00-00-05":
        time_5.append(item['time'] * 1000) # ms
        delay_table_5.append(item['e2edelay'] * 1000000) # us
    elif item['src'] == "00-00-00-00-00-07":
        time_7.append(item['time'] * 1000) # ms
        delay_table_7.append(item['e2edelay'] * 1000000) # us
    elif item['src'] == "00-00-00-00-00-08":
        time_8.append(item['time'] * 1000) # ms
        delay_table_8.append(item['e2edelay'] * 1000000) # us

# 计算延迟最大值，最小值，均值，方差
Max_1 = max(delay_table_1)
Max_3 = max(delay_table_3)
Max_4 = max(delay_table_4)
Max_5 = max(delay_table_5)
Max_7 = max(delay_table_7)
Max_8 = max(delay_table_8)
Min_1 = min(delay_table_1)
Min_3 = min(delay_table_3)
Min_4 = min(delay_table_4)
Min_5 = min(delay_table_5)
Min_7 = min(delay_table_7)
Min_8 = min(delay_table_8)
Avg_1 = np.mean(delay_table_1)
Avg_3 = np.mean(delay_table_3)
Avg_4 = np.mean(delay_table_4)
Avg_5 = np.mean(delay_table_5)
Avg_7 = np.mean(delay_table_7)
Avg_8 = np.mean(delay_table_8)
Std_1 = np.std(delay_table_1)
Std_3 = np.std(delay_table_3)
Std_4 = np.std(delay_table_4)
Std_5 = np.std(delay_table_5)
Std_7 = np.std(delay_table_7)
Std_8 = np.std(delay_table_8)
print("Demo 02")
print("ES1_avg: ", round(Avg_1,2), "us    ES1_std: " , round(Std_1,2), "us    ES1_max: ", round(Max_1,2), "us    ES1_min: ", round(Min_1,2),"us")
print("ES3_avg: ", round(Avg_3,2), "us    ES3_std: " , round(Std_3,2), "us    ES3_max: ", round(Max_3,2), "us    ES3_min: ", round(Min_3,2),"us")
print("ES4_avg: ", round(Avg_4,2), "us    ES4_std: " , round(Std_4,2), "us    ES4_max: ", round(Max_4,2), "us    ES4_min: ", round(Min_4,2),"us")
print("ES5_avg: ", round(Avg_5,2), "us    ES5_std: " , round(Std_5,2), "us    ES5_max: ", round(Max_5,2), "us    ES5_min: ", round(Min_5,2),"us")
print("ES7_avg: ", round(Avg_7,2), "us    ES7_std: " , round(Std_7,2), "us    ES7_max: ", round(Max_7,2), "us    ES7_min: ", round(Min_7,2),"us")
print("ES8_avg: ", round(Avg_8,2), "us    ES8_std: " , round(Std_8,2), "us    ES8_max: ", round(Max_8,2), "us    ES8_min: ", round(Min_8,2),"us")

# 绘制图片
# 限制图片大小
plt.figure(figsize=(20, 15))
# 绘制横向网格线
plt.grid(axis="y", which='both',linestyle=':')
# x轴设置
x1, x2 = 0, 550
dx = 50
plt.xticks(np.arange(x1,x2,dx),fontsize=16)
plt.xlabel('Time(ms)', fontsize=22)
# y轴设置
y1, y2 = 0, 35
dy = 5
plt.yticks(np.arange(y1,y2,dy),fontsize=16)
plt.ylabel('Traffic Delay(μs)', fontsize=22)



# 限制显示范围
plt.xlim(0,500)
plt.ylim(0.01,30)
# 剪裁图片
#plt.subplots_adjust(left=0.055, right=0.99, top=0.95, bottom=0.05)
# 画图
plt.scatter(time_1, delay_table_1, color = 'hotpink')
#plt.plot(time_1, delay_table_1, color='g', label='ES1', marker='^', ms=10, markevery=1)
plt.plot(time_3,delay_table_3,color='y',label='ES3')
plt.plot(time_4,delay_table_4,color='r',label='ES4')
plt.plot(time_5,delay_table_5,color='m',label='ES5')
plt.plot(time_7,delay_table_7,color='k',label='ES7')
plt.plot(time_8,delay_table_8,color='b',label='ES8')
# 图例设置
plt.legend(loc="upper right", fontsize=24)
# 显示图片
plt.show()





# # 设置子图横坐标
# ax[0].text(0.99, -0.1, 'time (ms)', fontsize=22, ha='right', va='bottom', transform=ax[0].transAxes)
# ax[1].text(0.99, -0.1, 'time (ms)', fontsize=22, ha='right', va='bottom', transform=ax[1].transAxes)


