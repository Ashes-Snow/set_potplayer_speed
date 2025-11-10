# 设置potplayer长按倍速
主要功能：一个C++的后台运行程序，通过按键钩子，做到长按->键倍速播放。

## 如何使用
首先，你需要有potplayer 播放器，如果没有，这是官方网站，可以[点击](https://potplayer.tv/?lang=zh_CN)前往下载最新版  
### 然后按照我的步骤设置  
1.如图所示，进入播放设置
<img width="989" height="977" alt="1" src="https://github.com/user-attachments/assets/f7c48db2-77e9-4f47-b3b6-90b2f13b95ed" />

2.然后键红框框中的速度调整单位改为2，即可实现长按3倍速，若想实现4倍速，则将数字改为3，其他倍速，以此类推  
<img width="588" height="691" alt="2" src="https://github.com/user-attachments/assets/b9a68541-d756-4780-802b-bcb1b0e8008e" />

3. 如图所示，进入时间跨度设置  
<img width="888" height="904" alt="3" src="https://github.com/user-attachments/assets/f0cba1d1-8dbe-4a96-adc4-0701fe83c82d" />

4.然后键红框框中的左右方向键改为0（一定要改为0），Ctrl+左右方向键改为5，即可实现使用脚本后按左/右方向键后，快进/快退5秒。  如果希望使用脚本后，单点左右方向键实现快进/快退其他的时间，则将5改为你希望快进/快退的时间  最下面的关键帧，建议取消勾选，勾选关键字可能会导致快进的时间不是5秒，而是参考关键帧的时间时间快进/快退。（关键帧并不一定时当前播放的帧，可能在2秒前，也肯能在5秒后，导致快进时间不固定为5秒。）  
<img width="579" height="492" alt="4" src="https://github.com/user-attachments/assets/4d87fdc8-0437-4ec8-a2e7-1b05827b3aa2" />

5. 按照你的操作系统，是64位还是32位，下载已经打包好的x64 或 x86 版本， 然后将其设为开机启动程序，这里采用创建计划任务的方式，如果你想使用其他方式，可以自行搜索

<img width="2445" height="1015" alt="image" src="https://github.com/user-attachments/assets/789ce72a-bd93-4729-b1f9-b9730ee181d3" />
<img width="870" height="686" alt="image" src="https://github.com/user-attachments/assets/fefd6857-73d9-4549-86c8-ef134459031b" />
<img width="867" height="646" alt="image" src="https://github.com/user-attachments/assets/553b0c16-21f9-4c5d-9c73-8ff1144cc5ce" />
这里注意，点击浏览，选择你将此可执行文件放置的位置  
<img width="867" height="646" alt="image" src="https://github.com/user-attachments/assets/73d28c40-3f3a-46ea-b599-b2fa8d043699" />
<img width="867" height="646" alt="image" src="https://github.com/user-attachments/assets/8537e89f-1cb2-4654-9e74-010d59e3afa6" />


如果你默认浏览器下载后自动运行，想要移送该文件，则需要先去任务管理器中关掉此进程。在任务管理器中搜索 potplayer_x64,右键管壁，即可移动该文件
<img width="1200" height="411" alt="image" src="https://github.com/user-attachments/assets/a54744b7-5b62-4061-b5e6-ed1a1e75e677" />


## 开发人员

源文件中包含本程序所有代码，可以复制去vs中打开，C++版本位C++17，也可以下载文件夹，然后直接使用 vs 打开这个项目

在v1.1版本代码中，请从vs2019升级到vs2022  
vs中，需要对项目进行如图设置：  
<img width="1883" height="1068" alt="属性设置" src="https://github.com/user-attachments/assets/548afe4d-f180-482b-b43b-43e0a04635f2" />


同时，由于彻底改为无控制台，所以无法直接进行调试，如果需要调试，请按照以下说明：
关于输出和调试信息：  

1. 在1.1中，我将std::cout 替换为 OutputDebugStringA，这样不会弹出控制台。可以用 Visual Studio 的输出窗口或 Sysinternals 的 DebugView 查看调试信息。
2. 如果你仍想保留控制台用于调试，可以改回 /SUBSYSTEM:CONSOLE 或在需要时临时添加 AllocConsole()。
