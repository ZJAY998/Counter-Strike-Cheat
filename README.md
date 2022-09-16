# Counter-Strike 透视自瞄教程

一款针对`Counter-Strike`反恐精英起源开发的带有动态菜单功能的`GDI`外部透视自瞄辅助，如下教程将为大家具体分析如何寻找基地址，以及透视算法如何实现，并应用已知数据实现一款透视自瞄辅助，该辅助已经在`Windows 10`系统下做过充分测试，稳定不闪烁。

外部D3D透视在团队竞技模式下，效果图如下：

![20220827102416](https://user-images.githubusercontent.com/52789403/187010712-4a9c4eb3-8ea7-4de8-9b09-175126a96559.png)

外部D3D透视在生化模式下，效果图如下：

![image](https://user-images.githubusercontent.com/52789403/190644699-615c9129-cfe4-4d96-b1c4-5cfd93b5dbb3.png)

内部D3D劫持透视在团队模式下，效果图如下：

![image](https://user-images.githubusercontent.com/52789403/190646423-beda566c-2134-43e7-ba19-539a5212dfd9.png)

方框透视的原理是通过读取游戏中已知坐标数据，并使用一定算法将自己与敌人之间的距离计算出来，结合`GDI绘图`函数在窗体上直接绘制图形，直到现在这种外挂依然具有极强的生命力，原因就是其比较通用，算法固定并能够应用于大部分的FPS游戏中。

<br>

GitHub 项目地址: https://github.com/lyshark/Counter-Strike-Cheat
