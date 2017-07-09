// Copyright (C) 2014-2017 Hideaki Narita


#ifndef IKURA_VARIABLEDIALOG_H
#define IKURA_VARIABLEDIALOG_H


#include <vector>
#include <gtkmm.h>


namespace hnrt
{
    //
    // Variable browser dialog box class
    //
    class VariableDialog : public Gtk::Dialog
    {
    public:

        VariableDialog();
        VariableDialog(Gtk::Window &parent);
        virtual ~VariableDialog() {}
        const Glib::ustring& getSelected() const { return selected; }

    protected:

        VariableDialog(const VariableDialog&) {}
        void init();
        void onSelectionChanged();
        void onAdded(const char *key, const char *value);
        void onChanged(const char *key, const char *value);

        class ListColumns : public Gtk::TreeModel::ColumnRecord
        {
        public:

            ListColumns()
            {
                add(colKey);
                add(colVal);
            }
            Gtk::TreeModelColumn<Glib::ustring> colKey;
            Gtk::TreeModelColumn<Glib::ustring> colVal;
        };

        Gtk::ScrolledWindow scrolledWindow;
        ListColumns columns;
        Gtk::TreeView treeView;
        Glib::RefPtr<Gtk::ListStore> store;
        Glib::ustring selected;
    };


    enum VariableDialogResponse
    {
        RESPONSE_COPY = 1,
        RESPONSE_PASTE = 2,
    };
}


#endif //!IKURA_VARIABLEDIALOG_H
