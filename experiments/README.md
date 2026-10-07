# 实验目录规范

## 结构

```
experiments/
├── README.md          # 本文件
├── baseline/          # 正常工况基线（开题后4周的硬节点交付）
│   ├── config.yaml    # 采样周期、轮询周期、节点数、阈值参数
│   ├── command.txt    # 完整复现命令
│   ├── notes.md       # 环境、接线、异常情况
│   └── metrics.csv    # 结果数据
├── exp01_fault_inject_sensor/   # 注入类型A：拔传感器接线 ×≥10
├── exp02_fault_inject_power/    # 注入类型B：节点断电 ×≥10
├── exp03_fault_inject_bus/      # 注入类型C：拔RS485线 ×≥10
└── exp04_perf_normal/           # 正常时段时延/丢包/任务响应
```

## 每个实验目录必须包含固定字段（写在该实验的 notes.md 首部）

```markdown
# Experiment ID
EXP-001

## Purpose
（本次实验要回答什么问题）

## Compared with
（与哪个 baseline / 哪个配置对比）

## Configuration
（config.yaml 的关键参数：采样周期、判据阈值、节点数）

## Dataset / 数据来源
（本人实验采集；数据路径；不含个人信息）

## Random seed
（如涉及随机性；确定性轮询实验写 N/A 并说明）

## Command
（复现命令，同步到 command.txt）

## Result
（指向 metrics.csv 与图表）

## Conclusion
（定量结论，带数字）

## Problems
（接触不良、异常记录——与注入故障分开记录）
```

## 故障注入实验强制要求

1. 至少 2 类故障（推荐 3 类），**每类重复注入 ≥10 次**。
2. 记录方法：上位机毫秒时间戳统一日志——
   - 注入动作：`INJECT,<type>,<seq>,<t_ms>`
   - 判据触发：`ALARM,<rule>,<node>,<t_ms>`
   - 检测时间 = t_alarm − t_inject
3. metrics.csv 每行一次注入：`seq,type,t_inject,t_alarm,detect_ms,missed(0/1),false_alarm(0/1),note`
4. 必须统计：检测时间均值与最大值、误报率、漏报率。
5. "接触不良导致的偶发数据丢失"单独打标签记录，**不计入注入故障样本**。
6. 正常时段另统计：通信时延（均值/最大/P95）、丢包率、任务响应时间（均值/最大抖动）。

## 状态总表（随实验更新）

| ID | 实验 | 注入次数 | 状态 |
|---|---|---|---|
| EXP-BASE | 正常工况基线 | — | 未开始 |
