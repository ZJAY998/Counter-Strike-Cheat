# Counter-Strike-Cheat

一款针对Counter-Strike反恐精英起源开发的带有动态菜单功能的GDI外部透视自瞄辅助，如下教程将为大家具体分析如何寻找基地址，以及透视算法如何实现，并应用已知数据实现一款透视自瞄辅助，最终效果图如下：

![20220827102416](https://user-images.githubusercontent.com/52789403/187010712-4a9c4eb3-8ea7-4de8-9b09-175126a96559.png)

方框透视的原理是通过读取游戏中已知坐标数据，并使用一定算法将自己与敌人之间的距离计算出来，结合GDI绘图函数在窗体上直接绘制图形，直到现在这种外挂依然具有极强的生命力，原因就是其比较通用，算法固定并能够应用于大部分的FPS游戏中。

### 方框透视算法分析

在前面的教程中我们已经手动找到了`FOV视场角`,`本人坐标数据`,`本人鼠标角度`,`敌人坐标数据`,`玩家数量`,`玩家是否死亡`,`敌人之间的数组偏移`,这些基址数据，多数情况下类FPS游戏找坐标手法都大同小异，接下来我们将具体分析计算方框的思路，以及实现这些方框绘制算法。

**第一象限求角：** 假设敌人在第一象限，求鼠标指向与敌人之间的夹角b，可以使用反正切求导。

![image](https://user-images.githubusercontent.com/52789403/187011755-0104c496-4768-491d-9e15-e76cee889d3e.png)

我们知道自己与敌人的相对(X,Y)距离，可以使用反正切公式求出a角的度数。而我们最终的目的是要求出我们的鼠标指向与敌人之间的夹角b，此时我们可以通过已知的鼠标角度C减去a既可得到b的角度。

**第二象限求角：** 假设敌人在第二象限，而我们的鼠标依然指向在第一象限，求敌人与X轴之间的夹角度数。

![image](https://user-images.githubusercontent.com/52789403/187011759-05e1e168-23b6-4dec-82fa-bf22e33a0008.png)

如上图：由于(X,Y)(黑色)是已知条件，我们可以通过X比Y求反正切，即可得到a角的度数，然后与90度相加，即可求出敌人当前坐标位置与X轴之间的夹角度数。

**第三四象限：** 敌人在第三与第四象限与上图差不多，最终目的就是求敌人的位置与X轴之间的夹角，第三象限应该加180度，第四象限加上270度数。这里就不罗嗦了，很简单的东西。

**另外4种特殊情况：** 如果敌人在第一象限且与X轴重合，那么敌人与X轴为之间的夹角度数必然为零度，同理如果与Y轴重合的话，那么X轴与敌人之间的夹角度数为90度，以此类推就是这四种特殊情况。

![image](https://user-images.githubusercontent.com/52789403/187011772-d2fc2988-21f1-4ef1-8b25-6277d3ac723e.png)

上方的4条象限与特殊情况，如果展开的话一共是8种不同的情况，如下代码就是这八种不同情况，调试下面的这段代码会发现一个缺陷，那就是当我们绕着敌人转圈时，偶尔会出现一个大于180度的角度，这又是两种非常特殊的情况。

![image](https://user-images.githubusercontent.com/52789403/187011775-5f7f108a-a15a-483e-964d-0454e52eb03e.png)

**特殊情况：** 当敌人在第四象限且鼠标角度依然在指向第一象限的情况下，则会出现大于180度的角。

![image](https://user-images.githubusercontent.com/52789403/187011784-f78b010b-aff3-4024-ad09-c1912836486f.png)

如上图：我们的目标是求鼠标角度与敌人之间的夹角度数，而此时的鼠标指向第一象限，而敌人却在第四象限上，我们用360度减去e角度（e = 敌人坐标与x轴之间的夹角度数），即可得到K角度，用K角度加上M角度，即可得到鼠标与敌人之间的夹角度数，另一种特殊情况敌人与鼠标角度调换位置求角，最终代码如下：

![image](https://user-images.githubusercontent.com/52789403/187011794-320610f4-c894-41e9-8b9f-6cfd8f1abf73.png)

**FOV视场角度：** 摄像机的作用就是，移动游戏中的场景，并将其投影到二维平面，显示给玩家。

![image](https://user-images.githubusercontent.com/52789403/187011805-86de7dd8-b016-4121-98da-e709f6be5111.png)

如上图：摄像机与屏幕之间的夹角统称为视场角，游戏中的准星位置到屏幕的边缘是FOV的一半，以屏幕分辨率`1024x768`为例，当FOV为`90度`时，则准心与屏幕的垂线构成`45度`等腰直角三角形，此时的摄像机到屏幕的距离就是一半屏幕长度`（1024/2 = 512）`的大小。

**三维横坐标转屏幕X坐标：** 将三维矩阵中的敌人坐标数据，转换为屏幕的X坐标。

![image](https://user-images.githubusercontent.com/52789403/187011816-a25b6f29-bd2c-4721-8fd3-45268766d001.png)

如上图：我们需要求出敌人位置的坐标数据，可以使用 `(x/y) x (1024/2) ` 最后还需要加上P的长度，由于窗口的总长度是1024那么我们可以直接除以2得到另一半的长度(512)，将敌人位置与另一半长度相加就是敌人投射在屏幕上的X坐标，但是此时我们并不知道(X,Y)的长度，所以需要先求出(X,Y) 如下图所示。

![image](https://user-images.githubusercontent.com/52789403/187011827-5c21e875-d0c3-4407-90c8-56517197d869.png)

上图中：我们需要求出`(X,Y)`的距离，此时我们已经知道了M和C的长度，则此时我们可以直接使用勾股定理`M的平方 + C的平方 （开方）= Z`，得到Z之后，通过 `sin a = (x/z) => sin a * z = X` 此时我们已经得到的X的长度，接着 `cos a =(y/z) => cos a * z = Y` 此时我们也得到了Y的长度，最后 `(x/y) x 512 + 512` 即可得到敌人位置，投射到屏幕上的X坐标。

**三维纵坐标转屏幕Y坐标：** 三维横坐标搞懂了，这个纵坐标就更简单了，如下图：

![image](https://user-images.githubusercontent.com/52789403/187011835-3bba8476-f98f-4496-ab3c-c742131beefb.png)

上图中：通过tan公式即可推导出d与c的距离，然后将d与c的长度相加，即可得到鼠标指向与敌人位置之间的距离，然后再加上屏幕高度的一半，本游戏屏幕高度为768，所以要加上384即可。

最终屏幕横坐标与纵坐标的转换算法如下所示，最后一点代码。

![image](https://user-images.githubusercontent.com/52789403/187011839-302870ed-6ebb-40fc-8f5c-68b4118ad9f2.png)
























<br><br>

原创首发地址: https://www.cnblogs.com/LyShark/p/11620244.html
