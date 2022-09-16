我们需要找到 DrawIndexedPrimitive 这个渲染函数并 Hook 这个函数，但 DrawIndexedPrimitive 函数与其他普通API函数不同，
由于 DirectX 的功能都是以COM组件的形式提供的类函数，所以普通的Hook无法搞它，
我这里的思路是，自己编写一个D3D绘图案例，在源码中找到 DrawIndexedPrimitive 函数并设置好断点，
通过VS调试单步执行找到函数的所在模块的地址，并与d3d9.dll的基址相减得到相对偏移地址。

首先我们直接在VS中运行自己的工程(这样的例子有很多)，然后在源代码中找到 `DrawIndexedPrimitive`并下一个【F9】断点，然后直接运行程序，发现程序断下后直接按下【Alt + 8】切到反汇编窗口。
```C
函数调用：g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 4);
00D01853 8B F4                mov         esi,esp  
00D01855 6A 04                push        4  
00D01857 6A 00                push        0  
00D01859 6A 04                push        4  
00D0185B 6A 00                push        0  
00D0185D 6A 00                push        0  
00D0185F 6A 04                push        4  
00D01861 A1 44 91 D0 00       mov         eax,dword ptr ds:[00D09144h]  
00D01866 8B 08                mov         ecx,dword ptr [eax]  
00D01868 8B 15 44 91 D0 00    mov         edx,dword ptr ds:[0D09144h]  
00D0186E 52                   push        edx  
00D0186F 8B 81 48 01 00 00    mov         eax,dword ptr [ecx+148h]  
00D01875 FF D0                call        eax  
00D01877 3B F4                cmp         esi,esp  
00D01879 E8 EF F8 FF FF       call        __RTC_CheckEsp (0D0116Dh) 
```
上方的代码就是你在VS中看到的代码片段，该代码片段就是调用 `DrawIndexedPrimitive` 函数的初始化工作，可以明显的看出压栈了6条数据，最后调用了 `call eax` 我们直接在单步【F9】走到00D01875地址处并按下【F11】进入到CALL的内部，可看到以下代码片段，我们需要记下片段中的 `6185CD20` 这个地址。
```C
6185CD20 8B FF                mov         edi,edi  
6185CD22 55                   push        ebp  
6185CD23 8B EC                mov         ebp,esp  
6185CD25 6A FF                push        0FFFFFFFFh  
6185CD27 68 C8 49 87 61       push        618749C8h  
6185CD2C 64 A1 00 00 00 00    mov         eax,dword ptr fs:[00000000h]  
6185CD32 50                   push        eax  
6185CD33 83 EC 20             sub         esp,20h  
6185CD36 53                   push        ebx  
6185CD37 56                   push        esi  
6185CD38 57                   push        edi  
6185CD39 A1 70 62 95 61       mov         eax,dword ptr ds:[61956270h]  
```
上方的起始地址 6185CD20 经常会变化，所以我们需要找到当前 d3d9.dll 模块的基址，通过X64DBG获取到的基址是`61800000`通过当前地址减去模块基址 `6185CD20 - 61800000` 得到相对偏移地址`5CD20`，此时我们就可以通过 d3d9.dll + 5CD20 来动态的计算出这个变化的地址
