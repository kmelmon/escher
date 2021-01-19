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
		void OnTick(Platform::Object^ sender, Platform::Object^ args);
		void LoadImages();
		Windows::Foundation::IAsyncOperation<Windows::Graphics::Imaging::SoftwareBitmap^>^ GenerateTileAsync(wchar_t* uri);
		Windows::Graphics::Imaging::SoftwareBitmap^ InvertTile(Windows::Graphics::Imaging::SoftwareBitmap^ tile);
		void InitializeTileGrid();
		void UpdateImageBrushes();

		void OnSizeChanged(Platform::Object^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ e);

	private:
		Platform::Collections::Vector<Windows::UI::Xaml::Media::Imaging::SoftwareBitmapSource^> m_tiles;
		Platform::Collections::Vector<Windows::UI::Xaml::Media::Imaging::SoftwareBitmapSource^> m_inverseTiles;
		Windows::UI::Xaml::DispatcherTimer^ m_timer;
		unsigned int m_frame = 0;
		unsigned int m_numFrames = 0;
	};
}
