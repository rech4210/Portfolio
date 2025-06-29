using UnrealBuildTool;
using System.IO;

public class DatabaseModule : ModuleRules
{
	public DatabaseModule(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"InventoryModule",
				"GameSharedModule"
			}
		);
			
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"HTTP",
				"Json",
				"JsonUtilities",
			}
		);

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// Add a definition to indicate we're building statically
			PublicDefinitions.Add("STATIC_CONCPP");
			PublicDefinitions.Add("CONCPP_BUILD_SHARED=0");
			// Base path to the MySQL Connector/C++ library
			string MySQLConnectorPath = Path.Combine(ModuleDirectory, "ThirdParty", "MySQLConnector");

			// Add the include path for private headers
			PrivateIncludePaths.Add(Path.Combine(MySQLConnectorPath, "include"));

			// Add the static library path
			string LibPath = Path.Combine(MySQLConnectorPath, "lib64", "vs14");
			PublicAdditionalLibraries.Add(Path.Combine(LibPath, "mysqlcppconn-static.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(LibPath, "libcrypto.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(LibPath, "libssl.lib"));
			
			// Symbol Import Library
			// For static linking, we still need the SSL DLLs at runtime.
			string cryptoDll = Path.Combine(MySQLConnectorPath, "lib64", "libcrypto-3-x64.dll");
			string sslDll = Path.Combine(MySQLConnectorPath, "lib64", "libssl-3-x64.dll");
			
			if (File.Exists(cryptoDll)) RuntimeDependencies.Add(cryptoDll);
			if (File.Exists(sslDll)) RuntimeDependencies.Add(sslDll);
		}
	}
} 

/** 1. 동적 라이브러리 방식 (mysqlcppconn.lib + mysqlcppconn-10-vs14.dll)
 *
 * - .lib 파일은 실행 코드를 포함하지 않고, DLL 참조 정보만 담고 있음 (Import Library).
 * - 컴파일 시 DLL의 함수들을 사용할 수 있도록 연결 정보를 제공함.
 * - 장점: 빌드 결과물이 작고, DLL 교체만으로도 버전 업데이트 가능.
 * - 단점: 실행 시 DLL이 반드시 필요하고, 누락 또는 버전 불일치 시 런타임 오류 발생.
 */

/** 2. 정적 라이브러리 방식 (mysqlcppconn-static.lib)
 *
 * - 정적 라이브러리는 Connector 코드 전체를 포함하며, 빌드 시 모듈 DLL 내부에 통합됨.
 * - 실행 시 별도의 DLL 없이도 작동함.
 * - 장점: 배포가 간단하고 DLL 관리 필요 없음.
 * - 단점: 모듈 크기 증가, Connector 교체 시 재빌드 필요.
 */

/** 3. Unreal Engine에서는 정적 라이브러리 방식이 유리한 이유
 *
 * - 모듈 단위로 DLL 하나만 배포하면 되므로 관리가 쉬움.
 * - DLL 파일 경로 설정이나 복사 없이도 런타임 안정성 확보.
 * - 로드 타이밍 문제나 DLL 누락 이슈를 원천 차단할 수 있음.
 */
