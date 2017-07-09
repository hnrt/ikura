// Copyright (C) 2014-2017 Hideaki Narita


#ifndef IKURA_MAINWINDOW_H
#define IKURA_MAINWINDOW_H


#include <vector>
#include <gtkmm.h>
#include "NumberDisplay.h"
#include "InputBuffer.h"
#include "HistoryBuffer.h"
#include "VariableDialog.h"


namespace hnrt
{
    class MainWindow : public Gtk::Window
    {
    public:

        MainWindow();
        virtual ~MainWindow();

    protected:

        MainWindow(const MainWindow&);
        void operator =(const MainWindow&);
        void onQuit();
        void onGoBackForward(int delta);
        void onCopy();
        void onPaste();
        void onZoomInOut(int delta);
        void onBrowseVariables();
        void onGroupingToggled();
        void onHexadecimalToggled();
        void onPrecisionChanged(int precision);
        void onAbout();
        void onSizeAllocate(Gtk::Allocation&);
        bool onKeyDown(GdkEventKey* event);
        bool onKeyUp(GdkEventKey* event);
        bool onMouseDown(GdkEventButton* event);
        bool onMouseUp(GdkEventButton* event);
        void onInput(guint key);
        void onTextChange(const char* s);
        void onTooltipChange(const char* s);
        void onClear();
        void onFirstChar();
        void onEvaluated(const char* expression, const char* value);
        void onInvalidChar();
        void onIncompleteExpression();
        void onDivisionByZero();
        void onOverflow();
        void onUnderflow();
        void onEvaluationInability();
        void onRecursiveVariableAccess(const char* key);
        void onVariableDialogResponse(int response);

        void beep() { get_window()->beep(); }

        void pasteFromHistory(size_t index);
        void onHistoryChange();

        void onClipboardOwnerChange(GdkEventOwnerChange* event);
        void onClipboardGet(Gtk::SelectionData& selectionData, guint info);
        void onClipboardClear();
        void onClipboardReceived(const Gtk::SelectionData& selectionData);
        void onClipboardReceivedTargets(const Glib::StringArrayHandle& targetsArray);
        void updatePasteStatus();
        void updateCopyStatus();

        Gtk::VBox box;
        Glib::RefPtr<Gtk::UIManager> uiManager;
        Glib::RefPtr<Gtk::ToggleAction> groupingAction;
        Glib::RefPtr<Gtk::ToggleAction> hexadecimalAction;
        Gtk::RadioButtonGroup precisionGroup;
        Glib::RefPtr<Gtk::RadioAction> noPrecisionAction;
        Glib::RefPtr<Gtk::RadioAction> precision10Action;
        Glib::RefPtr<Gtk::RadioAction> precision20Action;
        Gtk::HBox numberDisplayBox;
        NumberDisplay numberDisplay;
        Gtk::Table buttonTable;
        Gtk::Button button0;
        Gtk::Button button1;
        Gtk::Button button2;
        Gtk::Button button3;
        Gtk::Button button4;
        Gtk::Button button5;
        Gtk::Button button6;
        Gtk::Button button7;
        Gtk::Button button8;
        Gtk::Button button9;
        Gtk::Button buttonClear;
        Gtk::Button buttonEqual;
        Gtk::Button buttonPlus;
        Gtk::Button buttonMinus;
        Gtk::Button buttonMultiply;
        Gtk::Button buttonDivide;
        Gtk::Button buttonDecimal;
        Gtk::Button buttonBackspace;
        Gtk::Button buttonLeftParen;
        Gtk::Button buttonRightParen;
        Gtk::Button buttonExp;
        VariableDialog variableDialog;

        guint keyval;

        Glib::ustring clipboardStore;

        Glib::ustring appDisplayName;
        Glib::RefPtr<Gdk::Pixbuf> appIcon;

        int defaultWidth;
        int defaultHeight;
        int actualInitWidth;
        int actualInitHeight;
        int numberDisplayHPadding;
        int numberDisplayVPadding;
        Glib::ustring numberFontFamily;
        int numberFontSize;
        int buttonFontSize;

        InputBuffer input;
        HistoryBuffer history;
    };
}


#define APP_ICON_PATH "/usr/share/icons/gnome/32x32/apps/calc.png"


#endif //!IKURA_MAINWINDOW_H
