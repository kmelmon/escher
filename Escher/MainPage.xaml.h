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
		void LoadImages();
		Windows::Foundation::IAsyncOperation<Windows::Graphics::Imaging::SoftwareBitmap^>^ GenerateTileAsync();
		Windows::Graphics::Imaging::SoftwareBitmap^ InvertTile(Windows::Graphics::Imaging::SoftwareBitmap^ tile);
		void InitializeTileGrid();

		void OnSizeChanged(Platform::Object^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ e);

	private:
		Windows::Graphics::Imaging::SoftwareBitmap^ m_tile;
		Windows::UI::Xaml::Media::Imaging::SoftwareBitmapSource^ m_tileSource;
		Windows::Graphics::Imaging::SoftwareBitmap^ m_tileInverse;
		Windows::UI::Xaml::Media::Imaging::SoftwareBitmapSource^ m_tileInverseSource;
	};
}
