#pragma once

UENUM()
enum class EStringTableTextType : uint8
{
	UI,
	Max
};

/**
 * String Table에서 Text를 가져오는 전역 함수용 클래스입니다.
 * Localization Dashboard에서 String Table의 경로를 잡으면 Source String들이 자동으로 수집되기 때문에, 코드에서 하드코딩하는 것보다 편리합니다.
 * 언리얼에선 자동 수집, 언어 1:1 대응 UI, po파일 추출까지 지원합니다.
 * po파일을 모아 엑셀로 변환, 엑셀 파일을 다시 po파일로 변환하는 자동화 스크립트를 만들면 언어 통합 번역 시트를 제작할 수 있습니다.
 */
class FAuraTextManager
{
public:
	template <typename... ArgTypes>
	static FORCEINLINE FText GetText(EStringTableTextType Type, const FString& Key, ArgTypes... Args)
	{
		const FName TableId = GetPath(Type);
		FTextFormat Text = FText::FromStringTable(TableId, *Key);

		if (Text.GetSourceText().IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("Text가 비어있습니다. Table: %s / Key: %s"), *TableId.ToString(), *Key);
		}

		if constexpr (sizeof...(Args) > 0)
		{
			return FText::Format(MoveTemp(Text), MoveTemp(Args)...);
		}
		else
		{
			return Text.GetSourceText();
		}
	}

private:
	static FName GetPath(EStringTableTextType Type);
};
