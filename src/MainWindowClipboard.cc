// Copyright (C) 2014-2017 Hideaki Narita


#include <string.h>
#include "MainWindow.h"
#include "Expression.h"
#include "UTF8.h"


using namespace hnrt;


static const char UTF8_STRING[] = "UTF8_STRING";


void MainWindow::onCopy()
{
    Glib::RefPtr<Gtk::Clipboard> clipboard = Gtk::Clipboard::get();
    std::list<Gtk::TargetEntry> listTargets;
    listTargets.push_back(Gtk::TargetEntry(UTF8_STRING));
    clipboard->set(listTargets,
                   sigc::mem_fun(*this, &MainWindow::onClipboardGet),
                   sigc::mem_fun(*this, &MainWindow::onClipboardClear));
    if (input.size())
    {
        clipboardStore.assign(input, input.size());
    }
    else
    {
        clipboardStore.clear();
    }
    updatePasteStatus();
}


void MainWindow::onPaste()
{
    Glib::RefPtr<Gtk::Clipboard> clipboard = Gtk::Clipboard::get();
    clipboard->request_contents(UTF8_STRING, sigc::mem_fun(*this, &MainWindow::onClipboardReceived));
    updatePasteStatus();
}


void MainWindow::onClipboardOwnerChange(GdkEventOwnerChange* event)
{
    updatePasteStatus();
}


void MainWindow::onClipboardGet(Gtk::SelectionData& selectionData, guint info)
{
    const std::string target = selectionData.get_target();
    if (target == UTF8_STRING)
    {
        selectionData.set_text(clipboardStore);
    }
}


void MainWindow::onClipboardClear()
{
    clipboardStore.clear();
}


void MainWindow::onClipboardReceived(const Gtk::SelectionData& selectionData)
{
    const std::string target = selectionData.get_target();
    if (target == UTF8_STRING)
    {
        Glib::ustring clipboardData = selectionData.get_data_as_string();
        input.putString(clipboardData.c_str());
    }
}


void MainWindow::onClipboardReceivedTargets(const Glib::StringArrayHandle& targetsArray)
{
    std::list<std::string> targets = targetsArray;
    const bool bPasteIsPossible = std::find(targets.begin(), targets.end(), UTF8_STRING) != targets.end();
    Gtk::Widget* menubarPaste = uiManager->get_widget("/MenuBar/Edit/Paste");
    menubarPaste->set_sensitive(bPasteIsPossible);
    Gtk::Widget* toolbarPaste = uiManager->get_widget("/ToolBar/Paste");
    toolbarPaste->set_sensitive(bPasteIsPossible);
}


void MainWindow::updatePasteStatus()
{
    Glib::RefPtr<Gtk::Clipboard> clipboard = Gtk::Clipboard::get();
    clipboard->request_targets(sigc::mem_fun(*this, &MainWindow::onClipboardReceivedTargets));
}


void MainWindow::updateCopyStatus()
{
    bool bCopyIsPossible = input.size() ? true : false;
    Gtk::Widget* menubarCopy = uiManager->get_widget("/MenuBar/Edit/Copy");
    menubarCopy->set_sensitive(bCopyIsPossible);
    Gtk::Widget* toolbarCopy = uiManager->get_widget("/ToolBar/Copy");
    toolbarCopy->set_sensitive(bCopyIsPossible);
}
