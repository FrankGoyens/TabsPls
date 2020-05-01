using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

namespace LightSpeedExplorer
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class DirectoryView : Page
    {
        public DirectoryView()
        {
            this.InitializeComponent();
            FillDataGridWithFakeData();
        }

        private void FillDataGridWithFakeData()
        {
            this.FileListView.ItemsSource = new List<DirectoryEntry>
                {
                    new DirectoryEntry { Name = "India"},
                    new DirectoryEntry { Name = "South Africa"},
                    new DirectoryEntry { Name = "Nigeria"},
                    new DirectoryEntry { Name = "Singapore"}
                };
        }
    }
}