//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"

namespace Escher
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();

	protected:
		void InitializeTileGrid();

		void OnSizeChanged(Platform::Object^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ e);

		Windows::Foundation::IAsyncOperation<Windows::Graphics::Imaging::SoftwareBitmap^>^ GenerateTileAsync();

	};
}
