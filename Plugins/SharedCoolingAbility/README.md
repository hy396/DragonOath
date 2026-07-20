# SharedCoolingAbility
- 知乎链接:https://zhuanlan.zhihu.com/p/32216887423
- 案例链接:https://github.com/hbdjzwl/SharedCoolingTest  (插件支持4.27~5.5,Demo是5.5的版本)

在常规游戏中共享CD还是会经常会使用到的。比如魔兽世界的技能会有一个1秒的公共CD，某些游戏的同类药品、特殊道具再使用后会进入一个公共CD。一些MOBA或者RTS游戏也会遇见。

![Cooling_0](https://i.postimg.cc/pTkJ5t97/Cooling-0.png)

SharedCoolingAbility是一款简洁式开箱即用支持单机、联机的共享冷却插件，不需要你写一行代码，也不会耦合你的项目代码，只需要在自己的AbilitySystemComponent类继承一个接口和继承自共享冷却Ability即可实现公共CD。不管你是项目使用还是插件使用都非常的便捷。

![Cooling_1](https://i.postimg.cc/L4CVH2Fg/Cooling-1.png)

## 配置起来也极为方便

- 左图: 只需要在对应的GA类中配置相同的Tag与自定义时间
- 右图: 如果需要UI监听冷却的回调则使用下图右侧的两个Tag事件

![Cooling_4](https://i.postimg.cc/wMG5FcQQ/Cooling-4.png)



## 插件使用方法
### 1.安装插件
将插件下载并放入 ../Project/Plugins 项目插件目录下
![Cooling_14](https://i.postimg.cc/Tw2r6Hkc/Cooling-14.png)

### 2.打开插件
![Cooling_5](https://i.postimg.cc/C51Hj6kK/Cooling-5.png)

### 3.继承接口
- 方法一：直接使用插件里UASC_SharedCoolingComponent。
- 方法二：自定义AbilitySystemComponent然后继承自ISharedCoolingnterface。
  
![Cooling_6](https://i.postimg.cc/rF7Crjm8/Cooling-6.png)

> 如果你的项目已经用了AbilitySystemComponent，那么直接将你的AbilitySystemComponent继承自ISharedCoolingInterface即可。
如果你要在自己的项目中使用ISharedCoolingInterface，要记得添加插件的模块，步骤4会讲到。

### 4.创建GA
创建继承自GA_SharedCoolingBase的类都可以使用共享CD的功能。

![Cooling_7](https://i.postimg.cc/RZf1Qt6w/Cooling-7.png)

### 5.配置GA的冷却Tag
拥有同一个CD的GA配置同一个Tag，而后面的时间则代表引发GA对其它GA产生的共享冷却时间。

![Cooling_8](https://i.postimg.cc/QxFJCwj4/Cooling-8.png)

#### 参数讲解:
- bEnableSharedCooling ：配置共享冷却参数的开关。
- SharedCoolingTime : 是一个Map数组，Key是共享冷却的Tag，Value是共享冷却的时间。 
当GA_Shared_A激活时诱发的CD.SharedCooling.1会让其他拥有相同Tag的GA进入10秒的冷却。
当GA_Shared_B激活时诱发的CD.SharedCooling.1会让其他拥有相同Tag的GA进入20秒的冷却。
虽然是相同的Tag，但是不同的GA激活时候诱发CD的持续时间却不同。
- bSelfActivateDontSharedCoolDefaultConfig : 如果为true,当前激活GA的技能是不会被共享CD限制，其他拥有这个Tag的GA会进入冷却。（通俗来讲就是约束别人，不约束自己。）
- EventNotifyPlicy: 事件通知策略，当CD开始或结束时发起一个Event，可以是只通知客户端、或只通知服务端、也可以是双端都通知。（比如客户端接收到CD的通知，更新UI层面的表示）
> 该插件内置了Event.Cooling.Start和Event.Cooling.End两个Tag分别代表当冷却开始和冷却结束
![Cooling_13](https://i.postimg.cc/mkJM7ZTd/Cooling-13.png)


### 6.简单演示
创建3个共享GA，共享冷却分别为5、10、15秒
![Cooling_9](https://i.postimg.cc/HLgwSLXR/Cooling-9.png)

##### 点击第一个GA，会对其它GA产生5秒的共享CD，而第一个GA有默认GECD，且默认CD大于共享CD，则选择最大的冷却值。
![Cooling_10](https://i.postimg.cc/FRP0sg8X/Cooling-10.png)
![Cooling_11](https://i.postimg.cc/g0Hv1PKJ/Cooling-11.png)

##### 点击第三个GA，会对其它GA产生15秒的共享CD。
![Cooling_12](https://i.postimg.cc/rsKxdR20/Cooling-12.png)


## 番外
### 实例化策略(InstancingPolicy)
SharedCoolingAbility目前只支持InstancedPerActor策略。由于NonInstanced在5.5版本被废弃所以暂不支持NonInstanced方案。

![Cooling_11](https://i.postimg.cc/2yC38CKb/Cooling-15.png)


**【弊端】: 不能只谈好的一面**

- InstancedPerActor类似恶汉单例，会Give时预先创建好GA对象。当我们背包里拥有成千上万个不同的物品时就会导致创建大量的GA对象。当然一般背包不会有这么大，玩家也不会同时拥有这么多不同的物品，所以通常也就是在几十上百个左右物品是不需要担心的。

- 在原有支持NonInstanced(CDO)可以用其当做物品的GA使用，这样不管背包有多少个物品，成千上万，也不用担心会额外创建大量的GA对象。但是由于5.5的版本暂时废弃了，所以暂定，看官方后续版本会用什么代替。

- InstancedPerExecution的设定原本就与公共CD有设计歧义，所以不在考虑范围内。
  
GAS这套东西是支持单机联机的，匹配DS、LS、单机去使用。所以在有限人数的承载下不需要担心服务器有超额的设计。综合实际考虑用SharedCoolingAbility来做技能的共享CD或者物品道具的共享CD在实际游戏中也是较为合适的了。

### GameplayTag的注册方法
FGameplayTag的添加方法有很多，该插件使用的是第4种，案例演示用的第2种。
1. 配置文件DefaultGameplayTags.ini
2. 项目设置配置GameplayTagTableList表格
3. 宏UE_DEFINE_GAMEPLAY_TAG_STATIC
4. GameplayTagNativeAdder类AddTags函数

### 额外方法
额外提供了一些常用的方法，比如减少GE的冷却值、同时监听多个事件(不用写大量的WaitEvent)、常用的句柄比对、获取持续时间、调试字符串。

![Cooling_11](https://i.postimg.cc/QxbrKQtX/image.png)


### 版本差异

5.5的版本中官方对GAS做出了部分提示和修改，使用SharedCoolingAbility可能会有一些警告。


#### Replicated 变量的警告:

在4.27~5.4版本GA的Replicated变量都是支持复制的，到了5.5版本中会有以上警告。(但并不影响SharedCoolingAbility插件的使用)

![Cooling_55](https://i.postimg.cc/Gm8RYJRy/5-5.png)



从UE5.5开始，同步变量的使用已被弃用。弃用警告由控制台变量"AbilitySystem.DeprecateReplicatedProperties 0"控制，因此用户可以关闭警告并继续使用该功能，直到他们准备好解决问题。

![Cooling_55](https://i.postimg.cc/0yTsCM5H/image.png)

这样做的原因是为了防止用户遇到一个无法解决的关于同步顺序的错误：

- 同步变量保证会被传递，但不保证彼此之间或RPC函数的任何特定顺序。
- 游戏能力激活（以及大多数同步功能，如目标数据）依赖于客户端和服务器之间交换的RPC。
- 因此，当执行RPC（例如游戏能力激活）并对同步变量执行操作时，你永远不会保证拥有最新或过时的值。

-官方原文-
> ##### Replicated Variables in Gameplay Abilities
>
> The usage of replicated variables is deprecated as of UE5.5. The deprecation warning is controlled by a Console Variable "AbilitySystem.DeprecateReplicatedProperties", so that users can turn off the warning and continue using the feature until they are ready to fix the issue.|
>
> The reasoning is to prevent users from stumbling upon an impossible-to-solve bug regarding replication ordering:
>
> - Replicated variables are guaranteed to be delivered, but not in any particular order with respect to each other or RPC functions.
> - Gameplay Ability activation (and most synchronizing functions such as Target Data) rely on RPC's exchanged between the Client and Server.
> - Therefore, when executing an RPC (e.g. Gameplay Ability Activation) and performing operations on a replicated variable, you would never be guaranteed to have an up-to-date or stale value.
>
> For more information, see the [EDC article on object replication order](https://dev.epicgames.com/documentation/en-us/unreal-engine/replicated-object-execution-order-in-unreal-engine).
> If you believe you need a replicated variable, the solution is to instead use a Reliable RPC to send that data over. Using a Reliable RPC will ensure proper ordering with the underlying synchronization mechanisms of GAS.


##### NonInstanced 弃用 ：

官方觉得它不好用，不能网络同步、不能保存状态、建议使用InstancedPerActor。

![Cooling_11](https://i.postimg.cc/2yC38CKb/Cooling-15.png)

-官方原文-
> ###### NonInstanced (Deprecated)
>
> Prior to UE5.5, we had functionality for Non-Instanced Gameplay Abilities. Since these Gameplay Abilities were never instanced, they could not be replicated or even hold state (e.g. contain variables). All functions were called on the ClassDefaultObject and thus all state had to be held on the Gameplay Ability Spec. This made them very confusing to use. The same functionality can be achieved by simply using InstancedPerActor and never revoking it; the cost is just a single allocation (instance) of a UGameplayAbility.
