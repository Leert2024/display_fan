# 风扇全息显示屏图案生成

## 使用方法
1. 在根目录下新建`config.json`：
    ```json
    {
        "led_num": 20,
        "divide_num": 144,
        "pattern_files": [
            "pics/wo.png",
            "pics/ai.png",
            "pics/bei.png",
            "pics/you.png"
        ]
    }
    ```
    其中，`led_num`为一条半径（含圆心）上的LED数量；`divide_num`表示将整个圆分成多少份；`pattern_files`则是要生成的图案文件路径列表。图案切换顺序与`pattern_files`中的顺序一致。
2. 绘制想显示的图片：复制`pics/template.png`，在图中的黑色圆形区域绘制想显示的图片。
3. 运行main.py，自动将图片及相关信息转换成[C语言数组](../keil_project/Core/BUPT_RobotTeam_Libraries/hardware/fan/pattern.c)和[头文件](../keil_project/Core/BUPT_RobotTeam_Libraries/hardware/fan/pattern.h)的内容。