#include "EditorCamera.h"
#include "UnrealClient.h"
#include "Editor/EditorEngine.h"
#include "EditorViewportClient.h"

void UEditorCameraUtils::Get()
{
	auto ViewportClient = dynamic_cast<FEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient());
	Location = ViewportClient->GetViewLocation();
	Rotation = ViewportClient->GetViewRotation();
}

void UEditorCameraUtils::Set()
{
	auto ViewportClient = dynamic_cast<FEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient());
	ViewportClient->SetViewLocation(Location);
	ViewportClient->SetViewRotation(Rotation);
}
