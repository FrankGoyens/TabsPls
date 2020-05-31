using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks;
using TabsPlsCoreUWP;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Storage;
using Windows.System;
using Windows.UI.Popups;
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
    public sealed partial class DirectoryView : Page, INotifyPropertyChanged
    {
        public delegate void TabDisplayNameChangedHandler(string newName);

        public event TabDisplayNameChangedHandler TabDisplayNameChanged;
        public event PropertyChangedEventHandler PropertyChanged;

        public string TabDisplayName
        {
            get
            {
                if (currentDirectory == null)
                    return "<Loading directory>";
                return currentDirectory.DisplayName;
            }
        }
        public string DirectoryFieldContent { get; private set; }

        private ObservableCollection<DirectoryEntry> directoryItems = new ObservableCollection<DirectoryEntry>();
        private StorageFolder currentDirectory;
        private RobustDirectoryHistoryStore directoryHistoryStore = new RobustDirectoryHistoryStore();

        public DirectoryView()
        {
            this.InitializeComponent();
            FileListView.ItemsSource = directoryItems;

            FillDataGridWithPlaceholder();
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            var dir = e.Parameter as string;
            _ = ChangeDirectoryFromUserInput(dir);
        }

        private async Task<bool> ChangeDirectoryFromUserInput(string dir)
        {
            try 
            { 
                var storageFolder = await StorageFolder.GetFolderFromPathAsync(dir);
                ChangeDirectoryUsingStorageFolder(storageFolder);
                directoryHistoryStore.OnNewDirectory(storageFolder);
                return true;
            } 
            catch (System.IO.FileNotFoundException) { } 
            catch (System.ArgumentException) { } 
            catch (System.UnauthorizedAccessException) 
            {
                MessageDialog dlg = new MessageDialog(
                    "It seems you have not granted permission for this app to access the file system broadly. " +
                    "Without this permission, the app will only be able to access a very limited set of filesystem locations. " +
                    "You can grant this permission in the Settings app, if you wish. You can do this now or later. " +
                    "If you change the setting while this app is running, it will terminate the app so that the " +
                    "setting can be applied. Do you want to do this now?",
                    "File system permissions");
                dlg.Commands.Add(new UICommand("Yes", new UICommandInvokedHandler(InitMessageDialogHandler), 0));
                dlg.Commands.Add(new UICommand("No", new UICommandInvokedHandler(InitMessageDialogHandler), 1));
                dlg.DefaultCommandIndex = 0;
                dlg.CancelCommandIndex = 1;
                await dlg.ShowAsync();
            }
            catch (System.Exception) { }
            return false;
        }
        private void ChangeDirectoryUsingStorageFolder(StorageFolder storageFolder)
        {
            currentDirectory = storageFolder;
            DirectoryFieldContent = storageFolder.Path;
            FillDataGridWithDirectoryContents(storageFolder);

            TabDisplayNameChanged?.Invoke(TabDisplayName);
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(DirectoryFieldContent)));
        }

        private async void InitMessageDialogHandler(IUICommand command)
        {
            if ((int)command.Id == 0)
            {
                await Launcher.LaunchUriAsync(new Uri("ms-settings:privacy-broadfilesystemaccess"));
            }
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

        private async void DirectoryNavigationField_KeyDown(object sender, KeyRoutedEventArgs e)
        {
            if(e.Key == Windows.System.VirtualKey.Enter)
            {
                var textBox = sender as TextBox;
                if (!await ChangeDirectoryFromUserInput(textBox.Text))
                    textBox.Text = (await directoryHistoryStore.GetCurrent()).Path;
            }
        }

        private async void BackButton_Click(object sender, RoutedEventArgs e)
        {
            if (directoryHistoryStore.SwitchToPrevious())
                ChangeDirectoryUsingStorageFolder(await directoryHistoryStore.GetCurrent());
        }

        private async void ForwardButton_Click(object sender, RoutedEventArgs e)
        {
            if(directoryHistoryStore.SwitchToNext())
                ChangeDirectoryUsingStorageFolder(await directoryHistoryStore.GetCurrent());
        }
    }
}