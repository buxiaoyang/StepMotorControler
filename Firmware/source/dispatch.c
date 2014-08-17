#include <reg52.h>
#include <parameter.h>
#include <intrins.h>
#include <dispatch.h>

/*
按启动按钮，
气缸位置就绪， 送料气缸1启动，
碰到2号感应器，打料气缸2启动。
碰到4号感应器，夹紧气缸3启动。
碰到6号感应器，打料气缸2关闭。
碰到3号感应器，送料气缸1关闭。
碰到1号感应器，进刀气缸4启动。
碰到8号感应器，进刀气缸4关闭。
碰到7号感应器，夹紧气缸3关闭。
*/

unsigned int intervalTimerCount = 0;
unsigned int cylinderAlarmCount = 0;

unsigned char ManiDispatchSteps = 20;	// 1，1 3 5 7号感应器有效进入延迟
										// 2，延迟时间到，送料气缸1启动
										// 3，碰到2号感应器进入延迟
										// 4，延迟intervalTimer1时间到，打料气缸2启动
										// 5，碰到4号感应器进入延迟
										// 6，延迟intervalTimer2时间到，夹紧气缸3启动
										// 7，碰到6号感应器进入延迟
										// 8，延迟intervalTimer3时间到，打料气缸2关闭
										// 9，碰到3号感应器进入延迟
										// 10，延迟intervalTimer4时间到，送料气缸1关闭
										// 11，碰到1号感应器进入延迟
										// 12，延迟intervalTimer5时间到，进刀气缸4启动
										// 13，碰到8号感应器进入延迟
										// 14，延迟intervalTimer6时间到，进刀气缸4关闭
										// 15，碰到7号感应器进入延迟
										// 16，延迟intervalTimer7时间到，夹紧气缸3关闭
										// 20，空操作

void ManiDispatch(void)
{

}
