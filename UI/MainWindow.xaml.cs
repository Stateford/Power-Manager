using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Collections.ObjectModel;
using UIInterop;

namespace UI
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private ObservableCollection<String> ConfigProcesses;
        private ObservableCollection<Process> OpenProcesses;
        private Power PowerSettings;

        public MainWindow()
        {
            InitializeComponent();
            PowerSettings = new Power();

            ConfigProcesses = new ObservableCollection<String>();
            OpenProcesses = new ObservableCollection<Process>();

            RefreshData();
        
            LB_CONFIG_PROCESSES.ItemsSource = ConfigProcesses;
            LB_OPEN_PROCESSES.ItemsSource = OpenProcesses;
        }

        private void RefreshData()
        {
            var configProcesses = PowerSettings.getConfigWindows();

            ConfigProcesses.Clear();
            foreach(var process in configProcesses)
                ConfigProcesses.Add(process);

            var openProcesses = PowerSettings.getOpenProcesses().Where(p => !ConfigProcesses.Contains(p.ProcessName));

            OpenProcesses.Clear();

            foreach (var process in openProcesses)
                OpenProcesses.Add(process);
        }

        private void BTN_ENABLE_Click(object sender, RoutedEventArgs e)
        {
            var selectedItem = (Process)LB_OPEN_PROCESSES.SelectedItem;
            if (selectedItem != null)
            {
                PowerSettings.addProcess(selectedItem);
                RefreshData();
            }
        }

        private void BTN_REMOVE_PROCESS_Click(object sender, RoutedEventArgs e)
        {
            var selectedItem = (String)LB_CONFIG_PROCESSES.SelectedItem;
            if(selectedItem != null)
            {
                PowerSettings.removeProcess(selectedItem);
                RefreshData();
            }
        }
    }
}
