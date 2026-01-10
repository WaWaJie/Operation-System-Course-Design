# 操作系统课程设计 - 算法模拟实现
本项目是《操作系统》课程设计的实践成果，核心目标是对教材中经典的操作系统核心算法进行可视化模拟，直观展现算法执行逻辑与效果。
视频演示：https://www.bilibili.com/video/BV1aNiTBfEev/?spm_id_from=333.1387.homepage.video_card.click&vd_source=4084fc4444878e6b58c10043580d8a61
<img width="852" height="526" alt="OS课设要求" src="https://github.com/user-attachments/assets/1547ba9b-76a9-4244-aba9-8fc7f1b8224a" />

## 🛠️ 调试运行说明
若需在本地调试/运行项目，请严格遵循以下步骤：
1. 打开项目根目录下的 `.sln` 解决方案文件（适配 Visual Studio 2022 环境）；
2. **必须将编译模式从 Debug 切换为 Release 模式**（原因：项目仅配置了 Release 环境的依赖与编译参数，Debug 模式下会触发 “无声明” 类的编译报错）；
3. 确认环境配置无误后，即可正常编译运行。

![image](https://github.com/user-attachments/assets/74003fdf-ee7c-4a42-8623-cdb9c1c09053)

## 📖 项目说明
本项目仅针对《操作系统》教材中的核心算法做效果层面的模拟实现，所有逻辑均基于对课本内容的自主理解完成。由于是个人对理论知识点的解读与落地，部分功能的展现形式、实现细节可能并非严格契合操作系统的工业级标准。若各位发现项目中存在错误或不合理之处，欢迎随时指出，我会积极沟通并修正。

## ⚠️ 已知问题
项目中「内存分配」模块的可视化展示环节存在少量未修复的小 Bug，因时间与精力原因暂未优化（不影响核心算法逻辑的验证与理解）。

## 📝 学习资源
本人（WaWaJie）会将学习过程中的笔记整理发布在 CSDN 博客，目前已更新 OpenGL、设计模式等基础内容，感兴趣的同学可参考：
👉 [CSDN 博客专栏](https://blog.csdn.net/2301_79921853?type=blogColumn)

## 🙏 致谢
特别感谢我的项目组组员：
- 协助完成实验报告的撰写与梳理；
- 解决了开发过程中遇到的诸多问题与麻烦，让本项目能够顺利运行。

## 🌟 历史Star

[![Stargazers over time](https://starchart.cc/WaWaJie/Operation-System-Course-Design.svg?variant=adaptive)](https://starchart.cc/WaWaJie/Operation-System-Course-Design)

✨ 功能更新 | 2026.01.10
为拓展算法模拟的交互性与灵活性，本次更新引入解释器模式并集成 Lua 脚本运行能力，可以用来进行Lua的学习：
在对应功能模块中，支持自定义编写 Lua 代码并即时运行，可直观获取脚本执行结果；
绘图能力适配基础图形渲染：支持通过 Lua 脚本绘制填充式矩形、三角形、圆形，满足算法可视化的基础绘图需求。
<img width="1127" height="940" alt="image" src="https://github.com/user-attachments/assets/1abebb4a-107a-47f0-8645-c0a6ffbddf6d" />
<img width="1127" height="940" alt="image" src="https://github.com/user-attachments/assets/0841e7d2-77ab-40c5-b5d2-1b4693115de6" />


