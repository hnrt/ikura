// Copyright (C) 2014-2017 Hideaki Narita


#include <libintl.h>
#include "VariableDialog.h"
#include "VariableStore.h"
#include "UTF8.h"


using namespace hnrt;


VariableDialog::VariableDialog()
    : Gtk::Dialog(gettext("Variables"))
{
    init();
}


VariableDialog::VariableDialog(Gtk::Window &parent)
    : Gtk::Dialog(gettext("Variables"), parent)
{
    init();
}


void VariableDialog::init()
{
    Gtk::Button *copyButton = add_button(Gtk::Stock::COPY, RESPONSE_COPY);
    Gtk::Button *pasteButton = add_button(Gtk::Stock::PASTE, RESPONSE_PASTE);
    add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);
    set_default_response(Gtk::RESPONSE_CLOSE);

    copyButton->get_child()->set_tooltip_text(gettext("Keep the expression in the selected variable"));
    pasteButton->get_child()->set_tooltip_text(gettext("Paste the contents of the selected variable"));

    Gtk::VBox* box = get_vbox();

    scrolledWindow.add(treeView);
    scrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

    box->pack_start(scrolledWindow);

    store = Gtk::ListStore::create(columns);
    treeView.set_model(store);
    treeView.append_column(gettext("ID"), columns.colKey);
    treeView.append_column(gettext("Contents"), columns.colVal);
    treeView.get_column(0)->set_reorderable();
    treeView.get_column(1)->set_reorderable();
    Glib::RefPtr<Gtk::TreeSelection> selection = treeView.get_selection();
    selection->set_mode(Gtk::SELECTION_SINGLE);
    selection->signal_changed().connect(sigc::mem_fun(*this, &VariableDialog::onSelectionChanged));

    VariableStore::instance().signalAdd().connect(sigc::mem_fun(*this, &VariableDialog::onAdded));
    VariableStore::instance().signalChange().connect(sigc::mem_fun(*this, &VariableDialog::onChanged));

    show_all_children();

    onSelectionChanged(); // initialize selected field and copy and paste button's sensitivity
}


//
// This method is invoked by the selection of the tree view when the selection is changed.
//
void VariableDialog::onSelectionChanged()
{
    Glib::RefPtr<Gtk::TreeSelection> selection = treeView.get_selection();
    Gtk::TreeIter iter = selection->get_selected();
    if (iter)
    {
        // selected
        Gtk::TreeModel::Row row = *iter;
        selected = row[columns.colKey];
        Glib::ustring value = row[columns.colVal];
        set_response_sensitive(RESPONSE_COPY, selected.length() == 1 ? true : false);
        set_response_sensitive(RESPONSE_PASTE, !value.empty());
    }
    else
    {
        // unselected
        selected.clear();
        set_response_sensitive(RESPONSE_COPY, false);
        set_response_sensitive(RESPONSE_PASTE, false);
    }
}


//
// This method is invoked by VariableStore when a new variable is added.
//
void VariableDialog::onAdded(const char *key, const char *value)
{
    Gtk::TreeModel::Row row = *(store->append());
    row[columns.colKey] = Glib::ustring(key);
    row[columns.colVal] = UTF8::replaceArithmeticSignsWithAlternates(value);
}


//
// This method is invoked by VariableStore when the value of a variable is changed.
//
void VariableDialog::onChanged(const char *key, const char *value)
{
    Gtk::TreeIter iter = store->get_iter("0"); // point to first item
    while (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring keyNext = row[columns.colKey];
        if (keyNext == key)
        {
            row[columns.colVal] = UTF8::replaceArithmeticSignsWithAlternates(value);
            Glib::RefPtr<Gtk::TreeSelection> selection = treeView.get_selection();
            if (selection->is_selected(iter))
            {
                set_response_sensitive(RESPONSE_PASTE, *value ? true : false);
            }
            break;
        }
        iter++; // move to next item
    }
}
