# catos_simple
## 分支: feature-device

#### 介绍
简化版的catos，去掉复杂的部分进行重构

#### 软件架构
软件架构说明


#### 安装教程

1.  下载安装arm-none-gcc(此处用的是2020-q4的版本)，链接如下：

    https://developer.arm.com/downloads/-/gnu-rm

2.  将arm-none-gcc解压到catOS同级目录

3.  修改catOS/Build/arm_none_gcc/arm_none_gcc.config中的CROSS_COMPILE路径

4.  编译

    $cd catOS/workspace
    $make p=device

    编译完成后的生成文件位于catOS/workspace/output/simple_demo目录下


#### 使用说明

1.  xxxx
2.  xxxx
3.  xxxx

#### 参与贡献

1.  Fork 本仓库
2.  新建 Feat_xxx 分支
3.  提交代码
4.  新建 Pull Request

### 日志
2022/7/30
1. 将设备驱动框架应用在uart1串口上，并修改了cat_stdio.c中标准输出的函数，重定向为device_write


#### 特技

1.  使用 Readme\_XXX.md 来支持不同的语言，例如 Readme\_en.md, Readme\_zh.md
2.  Gitee 官方博客 [blog.gitee.com](https://blog.gitee.com)
3.  你可以 [https://gitee.com/explore](https://gitee.com/explore) 这个地址来了解 Gitee 上的优秀开源项目
4.  [GVP](https://gitee.com/gvp) 全称是 Gitee 最有价值开源项目，是综合评定出的优秀开源项目
5.  Gitee 官方提供的使用手册 [https://gitee.com/help](https://gitee.com/help)
6.  Gitee 封面人物是一档用来展示 Gitee 会员风采的栏目 [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)
