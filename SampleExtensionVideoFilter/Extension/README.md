# 实现 Agora SDK 提供的扩展点并利用 Agora SDK 的基本能力
  这里我们以 Video Filter 扩展点举例，目前 Agora 允许在视频编码前或解码后两个位置上引入 Video Filter 插件，实现定制的功能。

  Agora SDK 提供了三个接口：IVideoFilter, IExtensionProvider,IExtensionControl。
  class IVideoFilter 是负责具体处理 video frame 的接口，这个是 Extension 开发者要实现的接口，其中最重要的就是 adaptVideoFrame 方法，该方法处理 video frame, Video Filter 可以在这里实现诸如美颜，图像识别等功能。Extension 开发者还可以实现 setProperty，允许 APP 开发者动态设置插件属性。

  class IExtensionProvider 是一个工厂类接口，利用它可以创建 IVideoFilter 的实例。这也是 Extension 开发者要实现的。

  IExtensionControl 是 Agora SDK 给 Video Filter 使用的基本能力，Agora SDK 在调用 IExtensionProvider 创建 Video Filter 之前会将 IExtensionControl 传递给IExtensionProvider。IExtensionProvidre 在创建 Video Filter 时可以将 IExtensionControl 传递给 Video Filter 实例，Video Filter 可以利用 IExtensionControl 实现日志打印，事件通知等功能。

# 将 Extension 实现封装成 library 进行分发
  Extension 开发者需要将插件封装到 library 中，并告知 App 开发者如何在加载插件后获取到 IExtensionProvider 实例。