/////////////////////////////////////////////////////////////////////
// Executive.cpp - Client GUI for the Remote code publisher        //
// ver 1.4                                                         //
//-----------------------------------------------------------------//
// Rishit Reddy Muthyala (c) copyright 2016                         //
// All rights granted provided this copyright notice is retained   //
//-----------------------------------------------------------------//
// Language:    C++, Visual Studio 2015                            //
// Platform:    Dell XPS 8900, Windows 10                          //
// Application: Project #2, CSE687 - Object Oriented Design, S2015 //
// Author:     Rishit reddy Muthyala, Syracuse University, CST 4-187  //
/////////////////////////////////////////////////////////////////////



using System;
using System.IO;
using System.Windows;
using Microsoft.Win32;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Threading;

namespace WpfTutorialSamples.Dialogs
{
    public partial class OpenFileDialogMultipleFilesSample : Window
    {

        Shim shim = new Shim();
        Thread receieveThread = null;
        String SelectedFileCategory;
        String SelectedFile;
        List<String> fileList = new List<string>();

        public OpenFileDialogMultipleFilesSample()
        {
            InitializeComponent();

        }

        private void hanldeBrowseButton(object sender, RoutedEventArgs e)
        {
            GUIFileList.Items.Clear();
            fileList.Clear();
            OpenFileDialog openFileDialog = new OpenFileDialog();
            openFileDialog.Multiselect = true;
            openFileDialog.InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments);
            if (openFileDialog.ShowDialog() == true)
            {
                foreach (string filename in openFileDialog.FileNames)
                {

                    Console.WriteLine("The user wants to upload the file " + Path.GetFullPath(filename));
                    GUIFileList.Items.Add(Path.GetFileName(filename));
                    String filetoBeSent = Path.GetFileName(filename);
                    fileList.Add(Path.GetFullPath(filename));
                }
            }
        }

        private void ComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            ComboBox cmb = sender as ComboBox;
            SelectedFileCategory = cmb.SelectedItem.ToString();
            Console.WriteLine("The user has selected the category: " + SelectedFileCategory);
        }

        private void ComboBoxForFiles_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            ComboBox cmb = sender as ComboBox;
            SelectedFile = cmb.SelectedItem.ToString();
            Console.WriteLine("The user has selected the file: " + SelectedFile);
        }



        private void hanldeAnalyzeButton(object sender, RoutedEventArgs e)
        {
            Analyze.Background = Brushes.Green;

            shim.PostMessage("RunCodeAnalyzerOnServer");
        }


        private void uploadButtonClick(object sender, RoutedEventArgs e)
        {


            foreach (string str in fileList)
            {
                String message = "FileUploadToRepo" + str;
                shim.PostMessage("FileUploadToRepo" + str);
            }

        }


        private void handleGetCategoriesButton(object sender, RoutedEventArgs e)
        {
            shim.PostMessage("getcategories");
        }

        private void handleGetFilesButton(object sender, RoutedEventArgs e)
        {


            if (SelectedFileCategory.Length != 0)
            {

                shim.PostMessage("fetchFileNames" + "#" + SelectedFileCategory);

            }
        }
        private void hanldeDownloadFileButton(object sender, RoutedEventArgs e)
        {

            Console.WriteLine("Requesting Repo for the file: " + SelectedFile);
            if ((SelectedFileCategory != null) && (SelectedFile != null))
            {
                //shim.PostMessage("downloadFileHtmlContent" + "$" + SelectedFileCategory + "$" + SelectedFile);
                shim.PostMessage("downloadFileFromServer" + "#" + SelectedFileCategory + "#" + SelectedFile);
            }
            else
            {
                Console.WriteLine("Please select both the category and file name for download..");
            }
        }

        // delete files button
        private void handleDeleteFilesButton(object sender, RoutedEventArgs e)
        {
            Console.WriteLine("Requesting Repo to delete the file: " + SelectedFile);
            if ((SelectedFileCategory != null) && (SelectedFile != null))
            {
                shim.PostMessage("deleteFilesMessage" + "#" + SelectedFileCategory + "#" + SelectedFile);
            }
            else
            {
                Console.WriteLine("Please select both the category and file name for deletion..");
            }

        }


        void AddToComboBox(String msg)
        {

            try
            {

                List<string> listmaster = msg.Split('$').ToList();

                if (listmaster[0] == "getCategoriesReply")
                {


                    comboBox.Items.Clear();
                    List<string> list1 = listmaster[1].Split(',').ToList();
                    foreach (String str in list1)
                    {

                        comboBox.Items.Add(str);
                    }



                }
                else if (listmaster[0] == "getFileNamesReply")
                {

                    comboBox1.Items.Clear();
                    List<string> list1 = listmaster[1].Split(',').ToList();
                    foreach (String str in list1)
                    {

                        comboBox1.Items.Add(str);
                    }

                }

            }
            catch (Exception e)
            {
                Console.WriteLine("Exception caught while adding to Combobox: " + e.ToString());
            }
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {

            receieveThread = new Thread(
              () =>
              {
                  try
                  {
                      while (true)
                      {
                          String msg = shim.GetMessage();
                          Action<String> act = new Action<string>(AddToComboBox);
                          Object[] args = { msg };
                          Dispatcher.Invoke(act, args);
                          //  Console.WriteLine("Inside thread of Window.xaml.cs file message frm getMessage is " + msg);
                      }
                  }
                  catch (Exception exp)
                  {
                      Console.WriteLine("Exception caught in Window_Loaded: thread " + exp.ToString());
                  }
              }
            );
            receieveThread.Start();
        }
    }
}