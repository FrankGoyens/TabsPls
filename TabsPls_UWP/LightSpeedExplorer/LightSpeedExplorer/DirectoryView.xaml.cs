using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Storage;
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
        public delegate void TabDisplayNameChangedHandler(string newName);

        public event TabDisplayNameChangedHandler TabDisplayNameChanged;
        
        public string TabDisplayName
        {
            get
            {
                if (CurrentDirectory == null)
                    return "<Loading directory>";
                return CurrentDirectory.DisplayName;
            }
        }
        public string DirectoryFieldContent { get; private set; }
        private ObservableCollection<DirectoryEntry> directoryItems = new ObservableCollection<DirectoryEntry>();
        private StorageFolder CurrentDirectory;

        public DirectoryView()
        {
            this.InitializeComponent();
            FileListView.ItemsSource = directoryItems;

            FillDataGridWithPlaceholder();
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            var dir = e.Parameter as string;
            ChangeDirectory(dir);
        }

        private async void ChangeDirectory(string dir)
        {
            try
            {
                var storageFolder = await StorageFolder.GetFolderFromPathAsync(dir);
                CurrentDirectory = storageFolder;
                DirectoryFieldContent = storageFolder.Path;
                FillDataGridWithDirectoryContents(storageFolder);

                TabDisplayNameChanged?.Invoke(TabDisplayName);
            } catch (System.IO.FileNotFoundException) { } catch (System.UnauthorizedAccessException) { } catch (System.ArgumentException) { }

        }

        private async void FillDataGridWithDirectoryContents(StorageFolder storageFolder)
        {
            var contentsInFolder = await storageFolder.GetItemsAsync();

            directoryItems.Clear();
            foreach (var item in contentsInFolder)
                directoryItems.Add(new DirectoryEntry { Name = item.Name });
        }

        private void FillDataGridWithPlaceholder()
        {
            directoryItems.Clear();

            directoryItems.Add(new DirectoryEntry { Name = "<Folder not loaded>" });
        }
    }
}