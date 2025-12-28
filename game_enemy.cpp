#include "game_enemy.h"

// 生成敌人函数，随机选择路径
void spawnEnemy() {
    // 检查敌人数量限制
    if (game.enemyCount >= MAX_ENEMIES) return;
    // 检查当前波次是否已完成生成
    if (game.enemiesSpawned >= game.enemiesInWave) return;

    Enemy* e = &game.enemies[game.enemyCount];

    // 随机选择敌人类型（概率分布）
    int typeRand = rand() % 100;  // 0-99随机数
    EnemyType type;

    // 概率分布：基础40%，快速30%，坦克30%
    if (typeRand < 40) type = ENEMY_BASIC;     // 0-39：基础敌人
    else if (typeRand < 70) type = ENEMY_FAST; // 40-69：快速敌人
    else type = ENEMY_TANK;                    // 70-99：坦克敌人

    // 随机选择路径（0,1,2）
    int pathId = rand() % NUM_PATHS;

    // 设置起点位置（第一个路径点）
    e->gridX = game.paths[pathId][0].x;
    e->gridY = game.paths[pathId][0].y;
    e->x = e->gridX * GRID_SIZE + GRID_SIZE / 2;  // 像素坐标
    e->y = e->gridY * GRID_SIZE + GRID_SIZE / 2;
    e->pathIndex = 1;  // 下一个目标路径点索引
    e->pathId = pathId; // 记录使用的路径ID

    e->type = type;     // 敌人类型
    e->isAlive = 1;     // 设置为存活状态

    // 根据敌人类型设置具体属性
    switch (type) {
    case ENEMY_BASIC:  // 基础敌人
        e->speed = 0.8 * game.gameSpeed;  // 基础速度受游戏速度影响
        e->maxSpeed = 2;
        e->life = 50 + game.wave * 5;  // 生命随波次增加
        e->damage = 5;                 // 对基地伤害
        e->money = 10;                 // 击杀奖励
        e->color = RGB(150, 150, 150); // 灰色
        break;
    case ENEMY_FAST:   // 快速敌人
        e->speed = 1.5 * game.gameSpeed;
        e->maxSpeed = 3;
        e->life = 30 + game.wave * 3;
        e->damage = 3;
        e->money = 15;
        e->color = RGB(100, 200, 100); // 绿色
        break;
    case ENEMY_TANK:   // 坦克敌人
        e->speed = 0.5 * game.gameSpeed;
        e->maxSpeed = 1;
        e->life = 100 + game.wave * 10;
        e->damage = 10;
        e->money = 25;
        e->color = RGB(200, 100, 100); // 红色
        break;
    }

    e->maxLife = e->life;  // 设置最大生命值

    // 更新计数
    game.enemyCount++;       // 敌人总数增加
    game.enemiesSpawned++;   // 当前波次已生成敌人数增加
}

// 更新敌人移动，根据路径ID选择对应的路径
void updateEnemies() {
    // 遍历所有敌人
    for (int i = 0; i < game.enemyCount; i++) {
        Enemy* e = &game.enemies[i];
        // 跳过死亡的敌人
        if (!e->isAlive) continue;

        // 检查是否到达路径终点
        if (e->pathIndex >= game.pathCounts[e->pathId]) {
            // 到达终点，对基地造成伤害
            e->isAlive = 0;  // 敌人消失
            game.life -= e->damage;  // 减少基地生命

            // 检查游戏是否结束
            if (game.life <= 0) {
                game.life = 0;
                game.gameOver = 1;  // 游戏结束
                updateHighScores(); // 更新最高分记录
            }
            continue;  // 处理下一个敌人
        }

        // 获取当前目标路径点（根据路径ID）
        POINT target = game.paths[e->pathId][e->pathIndex];
        float targetX = target.x * GRID_SIZE + GRID_SIZE / 2;  // 转换为像素坐标
        float targetY = target.y * GRID_SIZE + GRID_SIZE / 2;

        // 计算当前位置到目标位置的向量
        float dx = targetX - e->x;
        float dy = targetY - e->y;
        float dist = sqrt(dx * dx + dy * dy);  // 计算距离

        // 如果到达目标路径点（距离小于2像素）
        if (dist < 2) {
            e->pathIndex++;  // 移动到下一个路径点
        }
        else {
            // 向目标位置移动
            e->x += (dx / dist) * e->speed;  // 归一化方向向量乘以速度
            e->y += (dy / dist) * e->speed;
        }
    }

    // 清理死亡的敌人（从数组中移除）
    for (int i = 0; i < game.enemyCount; i++) {
        if (!game.enemies[i].isAlive) {
            // 将后面的敌人向前移动，覆盖死亡的敌人
            for (int j = i; j < game.enemyCount - 1; j++) {
                game.enemies[j] = game.enemies[j + 1];
            }
            game.enemyCount--;  // 敌人计数减少
            i--;  // 重新检查当前位置（因为元素已移动）
        }
    }
}

// 绘制敌人（在drawGame函数中调用）
void drawEnemies() {
    for (int i = 0; i < game.enemyCount; i++) {
        Enemy* e = &game.enemies[i];

        // 绘制敌人身体（圆形）
        setfillcolor(e->color);
        solidcircle(e->x, e->y, GRID_SIZE / 3);

        // 绘制白色边框
        setlinecolor(RGB(255, 255, 255));
        circle(e->x, e->y, GRID_SIZE / 3);

        // 绘制生命条（红色）
        float lifeRatio = (float)e->life / e->maxLife;  // 计算生命比例
        setfillcolor(RGB(255, 0, 0));
        fillrectangle(e->x - GRID_SIZE / 2, e->y - GRID_SIZE / 2 - 5,
            e->x - GRID_SIZE / 2 + GRID_SIZE * lifeRatio,
            e->y - GRID_SIZE / 2 - 2);
    }
}