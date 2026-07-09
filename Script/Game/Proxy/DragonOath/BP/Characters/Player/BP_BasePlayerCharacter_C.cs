/*===========================================================================
    Generated code exported from UnrealCSharp.
    DO NOT modify this manually!
===========================================================================*/

using Script.CoreUObject;
using Interop;
using Script.DragonOath;
using Script.Engine;
using Script.Library;

namespace Script.Game.BP.Characters.Player
{
	[PathName("/Game/BP/Characters/Player/BP_BasePlayerCharacter.BP_BasePlayerCharacter_C")]
	public partial class BP_BasePlayerCharacter_C : ADOPlayerCharacter, IStaticClass
	{
		public UCameraComponent Camera
		{
			get
			{
				unsafe
				{
					var ReturnBuffer = stackalloc byte[8];

					FPropertyImplementation.FProperty_GetObjectPropertyImplementation(GarbageCollectionHandle, __Camera, ReturnBuffer);

					return (UCameraComponent)HandleData.GetObject(*(nint*)ReturnBuffer);
				}
			}

			set
			{
				unsafe
				{
					var InBuffer = stackalloc byte[8];

					*(nint*)InBuffer = value?.GarbageCollectionHandle ?? nint.Zero;

					FPropertyImplementation.FProperty_SetObjectPropertyImplementation(GarbageCollectionHandle, __Camera, InBuffer);
				}
			}
		}

		public USpringArmComponent SpringArm
		{
			get
			{
				unsafe
				{
					var ReturnBuffer = stackalloc byte[8];

					FPropertyImplementation.FProperty_GetObjectPropertyImplementation(GarbageCollectionHandle, __SpringArm, ReturnBuffer);

					return (USpringArmComponent)HandleData.GetObject(*(nint*)ReturnBuffer);
				}
			}

			set
			{
				unsafe
				{
					var InBuffer = stackalloc byte[8];

					*(nint*)InBuffer = value?.GarbageCollectionHandle ?? nint.Zero;

					FPropertyImplementation.FProperty_SetObjectPropertyImplementation(GarbageCollectionHandle, __SpringArm, InBuffer);
				}
			}
		}

		public new static UClass StaticClass()
		{
			return StaticClassSingleton ??= UObjectImplementation.UObject_StaticClassImplementation("/Game/BP/Characters/Player/BP_BasePlayerCharacter.BP_BasePlayerCharacter_C");
		}

		private static UClass StaticClassSingleton { get; set; }

		private static uint __Camera = 0;

		private static uint __SpringArm = 0;
	}
}