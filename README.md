# XHS 美颜插件
插件生成调试相关

### 编译步骤
* 目前只配了 Release x64，如有 Debug 需求自行配置
* 请使用 vs 打开 ExtensionVideoFilterTestDriver/ExtensionVideoFilterTestDriver.sln，里面有两个项目，设置 ExtensionVideoFilterTestDriver 为当前项目
* ExtensionVideoFilterTestDriver.cpp 里面的 GetAppId 方法，在对应的文件填入 appid
* 通过 Build-> Rebuild Solution 生成项目
* 运行程序，根据 ExtensionVideoFilterTestDriver.cpp 逻辑检查美颜效果

### agora sdk 更新
* 把新 sdk 的 highlevel + lowlevel 头文件拷贝到 Agora\include
* 把新 sdk 的 dll + lib 文件拷贝到 Agora\lib\x64
* 执行编译步骤

### 美颜库(x64)更新
* 把新库 dll/include/lib 拷到 SampleExtensionVideoFilter\XhsGraphics 相应目录
* 执行编译步骤，运行程序检查美颜是否正常
* 提示：调用美颜库相关逻辑所在文件：face_beauty_video_filter.cpp

### 编译细节
* 编译步骤里的 Build-> Rebuild Solution 过程(见两个项目各自的 Build Events 的 Pre-Build Event 和 Post-Build Event)
* ExtensionVideoFilterTestDriver 是测试程序(.exe)
* SampleExtensionVideoFilter 是插件(.dll)
* 会先编译 SampleExtensionVideoFilter 后编译 ExtensionVideoFilterTestDriver
* 编译插件时，生成 ExtensionVideoFilterTestDriver\x64\Release\XHSFaceBeautyExtension.dll
* 生成 SampleExtensionVideoFilter\package，里面包含 XHSFaceBeautyExtension.dll，以及从 SampleExtensionVideoFilter\XhsGraphics 拷贝的美颜库相关 dll，以及依赖的头文件，这个 package 也是最终给到客户的新的插件，每次 Rebuild 都会重新生成
* 生成 ExtensionVideoFilterTestDriver\x64\Release\ExtensionVideoFilterTestDriver.exe 测试程序
* 自动拷贝 package 里的插件和 SampleExtensionVideoFilter\XhsGraphics 的美颜测试资源到 ExtensionVideoFilterTestDriver\x64\Release\extensions\face_beauty.xhs
* 运行 ExtensionVideoFilterTestDriver.exe 检查美颜有无问题，如无问题，可把 package 里的文件打包作为新的插件包更新给客户
