// Copyright (C) 2014-2017 Hideaki Narita


#include <libintl.h>
#include "MainWindow.h"
#include "VariableStore.h"
#include "Expression.h"
#include "TerminalSymbol.h"
#include "OperatorInfo.h"
#include "LocaleInfo.h"
#include "UTF8.h"


//
// defined in /usr/include/X11/keysymdef.h
//
#define XK_BackSpace    0xff08  /* Back space, back char */
#define XK_Tab          0xff09
#define XK_Clear        0xff0b
#define XK_Return       0xff0d  /* Return, enter */
#define XK_Delete       0xffff  /* Delete, rubout */
#define XK_Left         0xff51  /* Move left, left arrow */
#define XK_Up           0xff52  /* Move up, up arrow */
#define XK_Right        0xff53  /* Move right, right arrow */
#define XK_Down         0xff54  /* Move down, down arrow */
#define XK_KP_Enter     0xff8d  /* Enter */
#define XK_KP_Left      0xff96
#define XK_KP_Up        0xff97
#define XK_KP_Right     0xff98
#define XK_KP_Down      0xff99
#define XK_KP_Prior     0xff9a
#define XK_KP_Next      0xff9b
#define XK_KP_Delete    0xff9f
#define XK_KP_Equal     0xffbd  /* Equals */
#define XK_KP_Multiply  0xffaa
#define XK_KP_Add       0xffab
#define XK_KP_Subtract  0xffad
#define XK_KP_Decimal   0xffae
#define XK_KP_Divide    0xffaf
#define XK_KP_0         0xffb0
#define XK_KP_1         0xffb1
#define XK_KP_2         0xffb2
#define XK_KP_3         0xffb3
#define XK_KP_4         0xffb4
#define XK_KP_5         0xffb5
#define XK_KP_6         0xffb6
#define XK_KP_7         0xffb7
#define XK_KP_8         0xffb8
#define XK_KP_9         0xffb9
#define XK_ISO_Left_Tab 0xfe20


using namespace hnrt;


static const char arithmeticOperators[] = "+-*/";


MainWindow::MainWindow()
    : box(false, 0)
    , numberDisplayBox(false, 0)
    , numberDisplay("0")
    , buttonTable(6, 4, true)
    , button0("0")
    , button1("1")
    , button2("2")
    , button3("3")
    , button4("4")
    , button5("5")
    , button6("6")
    , button7("7")
    , button8("8")
    , button9("9")
    , buttonClear("Clr")
    , buttonEqual("=")
    , buttonPlus("+")
    , buttonMinus(MINUS_SIGN)
    , buttonMultiply(MUL_SIGN)
    , buttonDivide(DIV_SIGN)
    , buttonDecimal(LocaleInfo::getDecimalPointString())
    , buttonBackspace("BS")
    , buttonLeftParen("(")
    , buttonRightParen(")")
    , buttonExp("e")
    , variableDialog(*this)
    , keyval(0)
    , appDisplayName(gettext("ikura"))
    , appIcon(Gdk::Pixbuf::create_from_file(APP_ICON_PATH))
    , defaultWidth(300)
    , defaultHeight(400)
    , actualInitWidth(-1)
    , actualInitHeight(-1)
    , numberDisplayHPadding(10)
    , numberDisplayVPadding(10)
    , numberFontFamily("Sans")
    , numberFontSize(20)
    , buttonFontSize(20)
{
    input.setGrouping(true);

    set_title(appDisplayName);
    set_default_size(defaultWidth, defaultHeight);
    set_icon(appIcon);

    add(box);

    Glib::RefPtr<Gtk::ActionGroup> actionGroup = Gtk::ActionGroup::create();

    actionGroup->add(Gtk::Action::create("File", gettext("_File")));
    actionGroup->add(Gtk::Action::create("Quit", Gtk::Stock::QUIT),
                     sigc::mem_fun(*this, &MainWindow::onQuit));

    actionGroup->add(Gtk::Action::create("Edit", gettext("_Edit")));
    actionGroup->add(Gtk::Action::create("Copy", Gtk::Stock::COPY),
                     sigc::mem_fun(*this, &MainWindow::onCopy));
    actionGroup->add(Gtk::Action::create("Paste", Gtk::Stock::PASTE),
                     sigc::mem_fun(*this, &MainWindow::onPaste));
    actionGroup->add(Gtk::Action::create("Backspace", gettext("Delete _last")),
                     Gtk::AccelKey("BackSpace"),
                     sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), XK_BackSpace));
    actionGroup->add(Gtk::Action::create("Clear", gettext("_Delete all")),
                     Gtk::AccelKey("Delete"),
                     sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), XK_Clear));
    actionGroup->add(Gtk::Action::create("Back", Gtk::Stock::GO_BACK, gettext("P_revious expression"), gettext("Previous expression")),
                     Gtk::AccelKey("<control>p"),
                     sigc::bind<int>(sigc::mem_fun(*this, &MainWindow::onGoBackForward), -1));
    actionGroup->add(Gtk::Action::create("Next", Gtk::Stock::GO_FORWARD, gettext("_Next expression"), gettext("Next expression")),
                     Gtk::AccelKey("<control>n"),
                     sigc::bind<int>(sigc::mem_fun(*this, &MainWindow::onGoBackForward), 1));
    actionGroup->add(Gtk::Action::create("Variables", Gtk::Stock::INDEX, gettext("Variables..."), gettext("Variables")),
                     Gtk::AccelKey("<control>m"),
                     sigc::mem_fun(*this, &MainWindow::onBrowseVariables));

    actionGroup->add(Gtk::Action::create("Insert", gettext("_Insert operator")));
    actionGroup->add(Gtk::Action::create("Abs", gettext("{abs}X ...absolute value of X")),
                     sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), SYM_ABS));
    actionGroup->add(Gtk::Action::create("Cbrt", gettext("{cbrt}X ...cube root of X")),
                     sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), SYM_CBRT));
    actionGroup->add(Gtk::Action::create("Cos", gettext("{cos}X ...cosine of X")),
                     sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), SYM_COS));
    actionGroup->add(Gtk::Action::create("Exp", gettext("{exp}X ...e raised to the power of X")),
                     sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), SYM_EXP));
    actionGroup->add(Gtk::Action::create("Hypot", gettext("X{hypot}Y ...euclidean distance; {sqrt}(X*X+Y*Y)")),
                     sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), SYM_HYPOT));
    actionGroup->add(Gtk::Action::create("Log", gettext("{log}X ...natural logarithm of X")),
                     sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), SYM_LOG));
    actionGroup->add(Gtk::Action::create("Log2", gettext("{log2}X ...base 2 logarithm of X")),
                     sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), SYM_LOG2));
    actionGroup->add(Gtk::Action::create("Log10", gettext("{log10}X ...base 10 logarithm of X")),
                     sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), SYM_LOG10));
    actionGroup->add(Gtk::Action::create("Pow", gettext("X{pow}Y ...X raised to the power of Y")),
                     sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), SYM_POW));
    actionGroup->add(Gtk::Action::create("Sin", gettext("{sin}X ...sine of X")),
                     sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), SYM_SIN));
    actionGroup->add(Gtk::Action::create("Sqrt", gettext("{sqrt}X ...square root of X")),
                     sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), SYM_SQRT));
    actionGroup->add(Gtk::Action::create("Tan", gettext("{tan}X ...tangent of X")),
                     sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), SYM_TAN));

    actionGroup->add(Gtk::Action::create("View", gettext("_View")));

    groupingAction = Gtk::ToggleAction::create("Grouping", gettext("Thousands' _grouping display"));
    groupingAction->set_active(input.getGrouping());
    actionGroup->add(groupingAction,
                     sigc::mem_fun(*this, &MainWindow::onGroupingToggled));

    hexadecimalAction = Gtk::ToggleAction::create("Hexadecimal", gettext("_Hexadecimal display"));
    hexadecimalAction->set_active(input.getHexadecimal());
    actionGroup->add(hexadecimalAction,
                     sigc::mem_fun(*this, &MainWindow::onHexadecimalToggled));

    noPrecisionAction = Gtk::RadioAction::create(precisionGroup, "NoPrecision", gettext("_Default precision display"));
    actionGroup->add(noPrecisionAction,
                     sigc::bind<int>(sigc::mem_fun(*this, &MainWindow::onPrecisionChanged), 0));
    precision10Action = Gtk::RadioAction::create(precisionGroup, "Precision10", gettext("Precision _10 display"));
    actionGroup->add(precision10Action,
                     sigc::bind<int>(sigc::mem_fun(*this, &MainWindow::onPrecisionChanged), 10));
    precision20Action = Gtk::RadioAction::create(precisionGroup, "Precision20", gettext("Precision _20 display"));
    actionGroup->add(precision20Action,
                     sigc::bind<int>(sigc::mem_fun(*this, &MainWindow::onPrecisionChanged), 20));
    switch (input.getPrecision())
    {
    case 10:
        precision10Action->set_active(true);
        break;
    case 20:
        precision20Action->set_active(true);
        break;
    default:
        noPrecisionAction->set_active(true);
        break;
    }

    actionGroup->add(Gtk::Action::create("ZoomIn", Gtk::Stock::ZOOM_IN, gettext("Use _larger font"), gettext("Larger font")),
                     Gtk::AccelKey("<control>l"),
                     sigc::bind<int>(sigc::mem_fun(*this, &MainWindow::onZoomInOut), 5));
    actionGroup->add(Gtk::Action::create("ZoomOut", Gtk::Stock::ZOOM_OUT, gettext("Use _smaller font"), gettext("Smaller font")),
                     Gtk::AccelKey("<control>s"),
                     sigc::bind<int>(sigc::mem_fun(*this, &MainWindow::onZoomInOut), -5));

    actionGroup->add(Gtk::Action::create("Help", gettext("_Help")));
    actionGroup->add(Gtk::Action::create("About", Gtk::Stock::ABOUT),
                     sigc::mem_fun(*this, &MainWindow::onAbout));

    uiManager = Gtk::UIManager::create();
    uiManager->insert_action_group(actionGroup);
    add_accel_group(uiManager->get_accel_group());

    Glib::ustring uiInfo =
        "<ui>"
        "  <menubar name='MenuBar'>"
        "    <menu name='File' action='File'>"
        "      <menuitem action='Quit'/>"
        "    </menu>"
        "    <menu name='Edit' action='Edit'>"
        "      <menu name='Insert' action='Insert'>"
        "        <menuitem name='Pow' action='Pow'/>"
        "        <menuitem name='Sqrt' action='Sqrt'/>"
        "        <menuitem name='Cbrt' action='Cbrt'/>"
        "        <menuitem name='Abs' action='Abs'/>"
        "        <menuitem name='Hypot' action='Hypot'/>"
        "        <menuitem name='Exp' action='Exp'/>"
        "        <menuitem name='Log' action='Log'/>"
        "        <menuitem name='Log2' action='Log2'/>"
        "        <menuitem name='Log10' action='Log10'/>"
        "        <menuitem name='Sin' action='Sin'/>"
        "        <menuitem name='Cos' action='Cos'/>"
        "        <menuitem name='Tan' action='Tan'/>"
        "      </menu>"
        "      <menuitem name='Variables' action='Variables'/>"
        "      <menuitem name='Copy' action='Copy'/>"
        "      <menuitem name='Paste' action='Paste'/>"
        "      <menuitem name='Backspace' action='Backspace'/>"
        "      <menuitem name='Clear' action='Clear'/>"
        "      <separator/>"
        "      <menuitem name='Back' action='Back'/>"
        "      <menuitem name='Next' action='Next'/>"
        "    </menu>"
        "    <menu name='View' action='View'>"
        "      <menuitem name='Grouping' action='Grouping'/>"
        "      <menuitem name='Hexadecimal' action='Hexadecimal'/>"
        "      <separator/>"
        "      <menuitem name='NoPrecision' action='NoPrecision'/>"
        "      <menuitem name='Precision10' action='Precision10'/>"
        "      <menuitem name='Precision20' action='Precision20'/>"
        "      <separator/>"
        "      <menuitem name='ZoomIn' action='ZoomIn'/>"
        "      <menuitem name='ZoomOut' action='ZoomOut'/>"
        "    </menu>"
        "    <menu name='Help' action='Help'>"
        "      <menuitem name='About' action='About'/>"
        "    </menu>"
        "  </menubar>"
        "  <toolbar name='ToolBar'>"
        "    <toolitem name='Back' action='Back'/>"
        "    <toolitem name='Next' action='Next'/>"
        "    <toolitem name='Copy' action='Copy'/>"
        "    <toolitem name='Paste' action='Paste'/>"
        "    <toolitem name='Variables' action='Variables'/>"
        "    <toolitem name='ZoomIn' action='ZoomIn'/>"
        "    <toolitem name='ZoomOut' action='ZoomOut'/>"
        "  </toolbar>"
        "</ui>";
    try
    {
        uiManager->add_ui_from_string(uiInfo);
    }
    catch (const Glib::Error& ex)
    {
        g_printerr("Error: uiManager->add_ui_from_string: %s\n", ex.what().c_str());
    }

    Gtk::Widget* menubar = uiManager->get_widget("/MenuBar");
    box.pack_start(*menubar, Gtk::PACK_SHRINK);

    Gtk::Widget* toolbar = uiManager->get_widget("/ToolBar");
    box.pack_start(*toolbar, Gtk::PACK_SHRINK);

    Gtk::Widget* copy = uiManager->get_widget("/ToolBar/Copy");
    copy->set_tooltip_text(gettext("Copy expression to Clipboard"));
    Gtk::Widget* paste = uiManager->get_widget("/ToolBar/Paste");
    paste->set_tooltip_text(gettext("Paste text from Clipboard"));

    numberDisplayBox.set_border_width(5);
    box.pack_start(numberDisplayBox, Gtk::PACK_SHRINK);

    Pango::FontDescription fontDesc;
    fontDesc.set_family(numberFontFamily);
    fontDesc.set_size(numberFontSize * PANGO_SCALE);

    numberDisplay.signal_key_press_event().connect(sigc::mem_fun(*this, &MainWindow::onKeyDown));
    numberDisplay.signal_key_release_event().connect(sigc::mem_fun(*this, &MainWindow::onKeyUp));
    numberDisplay.signal_button_press_event().connect(sigc::mem_fun(*this, &MainWindow::onMouseDown));
    numberDisplay.signal_button_release_event().connect(sigc::mem_fun(*this, &MainWindow::onMouseUp));
    numberDisplay.modify_font(fontDesc);
    numberDisplay.modify_bg(Gtk::STATE_NORMAL, Gdk::Color("white"));
    numberDisplay.set_padding(numberDisplayHPadding, numberDisplayVPadding);
    numberDisplayBox.pack_start(numberDisplay, Gtk::PACK_EXPAND_WIDGET);

    buttonClear.get_child()->set_tooltip_text(gettext("Delete all"));
    buttonBackspace.get_child()->set_tooltip_text(gettext("Delete last"));
    buttonExp.get_child()->set_tooltip_text(gettext("Exponent"));

    Pango::FontDescription buttonFontDesc;
    buttonFontDesc.set_family(numberFontFamily);
    buttonFontDesc.set_size(buttonFontSize * PANGO_SCALE);

    button0.get_child()->modify_font(buttonFontDesc);
    button1.get_child()->modify_font(buttonFontDesc);
    button2.get_child()->modify_font(buttonFontDesc);
    button3.get_child()->modify_font(buttonFontDesc);
    button4.get_child()->modify_font(buttonFontDesc);
    button5.get_child()->modify_font(buttonFontDesc);
    button6.get_child()->modify_font(buttonFontDesc);
    button7.get_child()->modify_font(buttonFontDesc);
    button8.get_child()->modify_font(buttonFontDesc);
    button9.get_child()->modify_font(buttonFontDesc);
    buttonClear.get_child()->modify_font(buttonFontDesc);
    buttonEqual.get_child()->modify_font(buttonFontDesc);
    buttonPlus.get_child()->modify_font(buttonFontDesc);
    buttonMinus.get_child()->modify_font(buttonFontDesc);
    buttonMultiply.get_child()->modify_font(buttonFontDesc);
    buttonDivide.get_child()->modify_font(buttonFontDesc);
    buttonDecimal.get_child()->modify_font(buttonFontDesc);
    buttonBackspace.get_child()->modify_font(buttonFontDesc);
    buttonLeftParen.get_child()->modify_font(buttonFontDesc);
    buttonRightParen.get_child()->modify_font(buttonFontDesc);
    buttonExp.get_child()->modify_font(buttonFontDesc);

    button0.signal_clicked().connect(sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), '0'));
    button1.signal_clicked().connect(sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), '1'));
    button2.signal_clicked().connect(sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), '2'));
    button3.signal_clicked().connect(sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), '3'));
    button4.signal_clicked().connect(sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), '4'));
    button5.signal_clicked().connect(sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), '5'));
    button6.signal_clicked().connect(sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), '6'));
    button7.signal_clicked().connect(sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), '7'));
    button8.signal_clicked().connect(sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), '8'));
    button9.signal_clicked().connect(sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), '9'));
    buttonClear.signal_clicked().connect(sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), XK_Clear));
    buttonEqual.signal_clicked().connect(sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), XK_Return));
    buttonPlus.signal_clicked().connect(sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), '+'));
    buttonMinus.signal_clicked().connect(sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), '-'));
    buttonMultiply.signal_clicked().connect(sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), '*'));
    buttonDivide.signal_clicked().connect(sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), '/'));
    buttonDecimal.signal_clicked().connect(sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), XK_KP_Decimal));
    buttonBackspace.signal_clicked().connect(sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), XK_BackSpace));
    buttonLeftParen.signal_clicked().connect(sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), '('));
    buttonRightParen.signal_clicked().connect(sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), ')'));
    buttonExp.signal_clicked().connect(sigc::bind<guint>(sigc::mem_fun(*this, &MainWindow::onInput), SYM_E));

    buttonTable.attach(buttonClear, 0, 1, 0, 1);
    buttonTable.attach(buttonLeftParen, 1, 2, 0, 1);
    buttonTable.attach(buttonRightParen, 2, 3, 0, 1);
    buttonTable.attach(buttonExp, 3, 4, 0, 1);
    buttonTable.attach(buttonBackspace, 0, 1, 1, 2);
    buttonTable.attach(buttonDivide, 1, 2, 1, 2);
    buttonTable.attach(buttonMultiply, 2, 3, 1, 2);
    buttonTable.attach(buttonMinus, 3, 4, 1, 2);
    buttonTable.attach(button7, 0, 1, 2, 3);
    buttonTable.attach(button8, 1, 2, 2, 3);
    buttonTable.attach(button9, 2, 3, 2, 3);
    buttonTable.attach(button4, 0, 1, 3, 4);
    buttonTable.attach(button5, 1, 2, 3, 4);
    buttonTable.attach(button6, 2, 3, 3, 4);
    buttonTable.attach(buttonPlus, 3, 4, 2, 4);
    buttonTable.attach(button1, 0, 1, 4, 5);
    buttonTable.attach(button2, 1, 2, 4, 5);
    buttonTable.attach(button3, 2, 3, 4, 5);
    buttonTable.attach(button0, 0, 2, 5, 6);
    buttonTable.attach(buttonDecimal, 2, 3, 5, 6);
    buttonTable.attach(buttonEqual, 3, 4, 4, 6);
    box.pack_start(buttonTable, Gtk::PACK_EXPAND_WIDGET);

    signal_size_allocate().connect(sigc::mem_fun(*this, &MainWindow::onSizeAllocate));

    // Although buttons process some keys such as Enter when they have input focus,
    // other keys need to be handled by this top level window for users' convenience.
    signal_key_press_event().connect(sigc::mem_fun(*this, &MainWindow::onKeyDown));
    signal_key_release_event().connect(sigc::mem_fun(*this, &MainWindow::onKeyUp));

    input.clear();
    input.signalTextChange().connect(sigc::mem_fun(*this, &MainWindow::onTextChange));
    input.signalTooltipChange().connect(sigc::mem_fun(*this, &MainWindow::onTooltipChange));
    input.signalClear().connect(sigc::mem_fun(*this, &MainWindow::onClear));
    input.signalFirstChar().connect(sigc::mem_fun(*this, &MainWindow::onFirstChar));
    input.signalEvaluated().connect(sigc::mem_fun(*this, &MainWindow::onEvaluated));
    input.signalInvalidChar().connect(sigc::mem_fun(*this, &MainWindow::onInvalidChar));
    input.signalIncompleteExpression().connect(sigc::mem_fun(*this, &MainWindow::onIncompleteExpression));
    input.signalDivisionByZero().connect(sigc::mem_fun(*this, &MainWindow::onDivisionByZero));
    input.signalOverflow().connect(sigc::mem_fun(*this, &MainWindow::onOverflow));
    input.signalUnderflow().connect(sigc::mem_fun(*this, &MainWindow::onUnderflow));
    input.signalEvaluationInability().connect(sigc::mem_fun(*this, &MainWindow::onEvaluationInability));
    input.signalRecursiveVariableAccess().connect(sigc::mem_fun(*this, &MainWindow::onRecursiveVariableAccess));

    history.clear();
    onHistoryChange();
    history.signalContentsChange().connect(sigc::mem_fun(*this, &MainWindow::onHistoryChange));
    history.signalIndexChange().connect(sigc::mem_fun(*this, &MainWindow::onHistoryChange));

    VariableStore& vars = VariableStore::instance();
    for (int c = 'A'; c <= 'Z'; c++)
    {
        char key[2];
        key[0] = c;
        key[1] = 0;
        vars.add(key);
    }
    vars.add("PI", "3.1415926535897932384626433832795029");
    vars.add("E$", "2.7182818284590452353602874713526625");
    vars.add("SHRT_MIN", "-32768");
    vars.add("SHRT_MAX", "32767");
    vars.add("USHRT_MAX", "65535");
    vars.add("INT_MIN", "-2147483648");
    vars.add("INT_MAX", "2147483647");
    vars.add("UINT_MAX", "4294967295");
    vars.add("LONG_MIN", "-9223372036854775808");
    vars.add("LONG_MAX", "9223372036854775807");
    vars.periodToDecimalPoint();

    variableDialog.signal_response().connect(sigc::mem_fun(*this, &MainWindow::onVariableDialogResponse));

    updateCopyStatus();
    updatePasteStatus();

    show_all_children();

    numberDisplay.grab_focus();
}


MainWindow::~MainWindow()
{
}


void MainWindow::onQuit()
{
    hide();
}


// delta for history index should be either -1 or +1
void MainWindow::onGoBackForward(int delta)
{
    pasteFromHistory(history.getIndex() + delta);
}


// delta for font size should be either -5 or +5
void MainWindow::onZoomInOut(int delta)
{
    const int minSize = 10;
    const int maxSize = 40;
    int newSize = numberFontSize + delta;
    if (minSize <= newSize && newSize <= maxSize)
    {
        numberFontSize = newSize;
        Pango::FontDescription fontDesc;
        fontDesc.set_family(numberFontFamily);
        fontDesc.set_size(numberFontSize * PANGO_SCALE);
        numberDisplay.modify_font(fontDesc);
        uiManager->get_widget("/MenuBar/View/ZoomIn")->set_sensitive(numberFontSize < maxSize ? true : false);
        uiManager->get_widget("/MenuBar/View/ZoomOut")->set_sensitive(numberFontSize > minSize ? true : false);
        uiManager->get_widget("/ToolBar/ZoomIn")->set_sensitive(numberFontSize < maxSize ? true : false);
        uiManager->get_widget("/ToolBar/ZoomOut")->set_sensitive(numberFontSize > minSize ? true : false);
    }
}


void MainWindow::onBrowseVariables()
{
    if (variableDialog.is_visible())
    {
        variableDialog.hide();
    }
    else
    {
        const int gap = 10;
        Glib::RefPtr<const Gdk::Screen> screen = get_screen();
        int scx = screen->get_width();
        int x = 0, y = 0, cx = 0, cy = 0;
        get_position(x, y);
        get_size(cx, cy);
        int dcx = 0, dcy = 0;
        variableDialog.get_size(dcx, dcy);
        // place dialog on the right size of this window when it is not clipped at the right edge of the screen
        if (x + cx + gap + dcx <= scx)
        {
            x += cx + gap;
        }
        // place dialog on the left size of this window when it is not clipped at the left edge of the screen
        else if (x - gap - dcx >= 0)
        {
            x -= gap + dcx;
        }
        // place dialog at the left edge of the screen
        else
        {
            x = 0;
        }
        variableDialog.move(x, y);
        variableDialog.show();
    }
}


void MainWindow::onGroupingToggled()
{
    input.setGrouping(groupingAction->get_active());
    if (input.isJustEvaluated())
    {
        pasteFromHistory(history.size() - 1);
        input.evaluate();
    }
}


void MainWindow::onHexadecimalToggled()
{
    input.setHexadecimal(hexadecimalAction->get_active());
    if (input.isJustEvaluated())
    {
        pasteFromHistory(history.size() - 1);
        input.evaluate();
    }
}


void MainWindow::onPrecisionChanged(int precision)
{
    switch (precision)
    {
    case 0:
        if (!noPrecisionAction->get_active())
        {
            return;
        }
        break;
    case 10:
        if (!precision10Action->get_active())
        {
            return;
        }
        break;
    case 20:
        if (!precision20Action->get_active())
        {
            return;
        }
        break;
    default:
        return;
    }
    input.setPrecision(precision);
    if (input.isJustEvaluated())
    {
        pasteFromHistory(history.size() - 1);
        input.evaluate();
    }
}


void MainWindow::onAbout()
{
    Glib::ustring copyright = "Copyright \xC2\xA9 2014-2017 ";
    copyright += gettext("Hideaki Narita");
    Gtk::AboutDialog dialog;
    dialog.set_icon(appIcon);
    dialog.set_logo(appIcon);
    dialog.set_program_name(appDisplayName);
    dialog.set_copyright(copyright);
    dialog.set_comments(gettext("A handy desktop calculator that can evaluate even a complex expression."));
    // put this dialog at the center of the main window
    int x = 0, y = 0, cx = 0, cy = 0;
    get_position(x, y);
    get_size(cx, cy);
    int dcx = 0, dcy = 0;
    dialog.get_size(dcx, dcy);
    x += (cx - dcx) / 2;
    y += (cy - dcy) / 2;
    dialog.move(x, y);
    // show in modal mode
    dialog.run();
}


void MainWindow::onSizeAllocate(Gtk::Allocation& a)
{
    if (actualInitWidth < 0 || actualInitHeight < 0)
    {
        actualInitWidth = a.get_width() >= defaultWidth ? a.get_width() : defaultWidth;
        actualInitHeight = a.get_height() >= defaultHeight ? a.get_height() : defaultHeight;
        set_size_request(actualInitWidth, actualInitHeight);
        variableDialog.set_default_size(actualInitWidth, actualInitHeight);
    }
}


bool MainWindow::onKeyDown(GdkEventKey* event)
{
    switch (event->keyval)
    {
    case XK_Tab:
    case XK_Down:
    case XK_KP_Down:
        child_focus(Gtk::DIR_TAB_FORWARD);
        break;
    case XK_ISO_Left_Tab:
    case XK_Up:
    case XK_KP_Up:
        child_focus(Gtk::DIR_TAB_BACKWARD);
        break;
    default:
        onInput(event->keyval);
        if (!numberDisplay.has_focus())
        {
            numberDisplay.grab_focus();
        }
        break;
    }
    return true;
}


bool MainWindow::onKeyUp(GdkEventKey* event)
{
    return true;
}


bool MainWindow::onMouseDown(GdkEventButton* event)
{
    numberDisplay.grab_focus();
    return true;
}


bool MainWindow::onMouseUp(GdkEventButton* event)
{
    return true;
}


void MainWindow::onInput(guint key)
{
    switch (key)
    {
    case '0':
    case XK_KP_0:
        input.putChar('0');
        break;
    case '1':
    case XK_KP_1:
        input.putChar('1');
        break;
    case '2':
    case XK_KP_2:
        input.putChar('2');
        break;
    case '3':
    case XK_KP_3:
        input.putChar('3');
        break;
    case '4':
    case XK_KP_4:
        input.putChar('4');
        break;
    case '5':
    case XK_KP_5:
        input.putChar('5');
        break;
    case '6':
    case XK_KP_6:
        input.putChar('6');
        break;
    case '7':
    case XK_KP_7:
        input.putChar('7');
        break;
    case '8':
    case XK_KP_8:
        input.putChar('8');
        break;
    case '9':
    case XK_KP_9:
        input.putChar('9');
        break;
    case '+':
    case XK_KP_Add:
        input.putChar('+');
        break;
    case '-':
    case XK_KP_Subtract:
        input.putChar('-');
        break;
    case '/':
    case XK_KP_Divide:
        input.putChar('/');
        break;
    case '*':
    case XK_KP_Multiply:
        input.putChar('*');
        break;
    case XK_KP_Decimal:
        input.putChar(LocaleInfo::getDecimalPoint());
        break;
    case SYM_E:
        input.putChar(SYM_E);
        break;
    case '(':
        input.putChar('(');
        break;
    case ')':
        input.putChar(')');
        break;
    case XK_Return:
    case XK_KP_Enter:
    case XK_KP_Equal:
        input.evaluate();
        break;
    case XK_Delete:
    case XK_Clear:
    case XK_KP_Delete:
        input.clear();
        break;
    case XK_BackSpace:
        input.deleteLastChar();
        break;
    case XK_Left:
    case XK_KP_Left:
    case XK_KP_Prior:
    {
        double value = numberDisplay.getAdjustmentValue();
        double valueMin = numberDisplay.getAdjustmentLower();
        value -= numberDisplay.getAdjustmentStep();
        if (value < valueMin)
        {
            value = valueMin;
        }
        numberDisplay.setAdjustmentValue(value);
        break;
    }
    case XK_Right:
    case XK_KP_Right:
    case XK_KP_Next:
    {
        double value = numberDisplay.getAdjustmentValue();
        double valueMax = numberDisplay.getAdjustmentUpper();
        value += numberDisplay.getAdjustmentStep();
        if (value > valueMax)
        {
            value = valueMax;
        }
        numberDisplay.setAdjustmentValue(value);
        break;
    }
    case '{':
        input.putChar('{');
        break;
    case '}':
        input.putChar('}');
        break;
    case SYM_ABS:
    case SYM_CBRT:
    case SYM_COS:
    case SYM_EXP:
    case SYM_HYPOT:
    case SYM_LOG:
    case SYM_LOG2:
    case SYM_LOG10:
    case SYM_POW:
    case SYM_SIN:
    case SYM_SQRT:
    case SYM_TAN:
        input.putString(OperatorInfo::instance().find((TerminalSymbol)key));
        break;
    case '=':
        input.putChar('=');
        break;
    default:
        if (key == (guint)LocaleInfo::getDecimalPoint())
        {
            input.putChar(LocaleInfo::getDecimalPoint());
        }
        else if (!(key & ~0xFF) && (isalpha(key) || key == '_' || key == '$' || key == '@'))
        {
            input.putChar(toupper(key));
        }
        break;
    }
}


void MainWindow::onTextChange(const char* s)
{
    numberDisplay.set_text(UTF8::replaceArithmeticSignsWithAlternates(s));
}


void MainWindow::onTooltipChange(const char* s)
{
    numberDisplay.set_tooltip_text(UTF8::replaceArithmeticSignsWithAlternates(s));
}


void MainWindow::onClear()
{
    history.moveIndexToEnd();
    updateCopyStatus();
}


void MainWindow::onFirstChar()
{
    updateCopyStatus();
}


void MainWindow::onEvaluated(const char* expression, const char* value)
{
    history.push_back(expression);
    (void)value;
}


void MainWindow::onInvalidChar()
{
    beep();
}


void MainWindow::onIncompleteExpression()
{
    beep();
}


void MainWindow::onDivisionByZero()
{
    beep();
    Gtk::MessageDialog dialog(*this,
                              gettext("Division by zero.\nModify the expression and try again."),
                              false,
                              Gtk::MESSAGE_WARNING);
    dialog.set_title(appDisplayName);
    dialog.run();
}


void MainWindow::onOverflow()
{
    beep();
    Gtk::MessageDialog dialog(*this,
                              gettext("Overflow occurred.\nModify the expression and try again."),
                              false,
                              Gtk::MESSAGE_WARNING);
    dialog.set_title(appDisplayName);
    dialog.run();
}


void MainWindow::onUnderflow()
{
    beep();
    Gtk::MessageDialog dialog(*this,
                              gettext("Underflow occurred.\nModify the expression and try again."),
                              false,
                              Gtk::MESSAGE_WARNING);
    dialog.set_title(appDisplayName);
    dialog.run();
}


void MainWindow::onEvaluationInability()
{
    beep();
    Gtk::MessageDialog dialog(*this,
                              gettext("Unable to calculate value.\nModify the expression and try again."),
                              false,
                              Gtk::MESSAGE_WARNING);
    dialog.set_title(appDisplayName);
    dialog.run();
}


void MainWindow::onRecursiveVariableAccess(const char* key)
{
    Glib::ustring message = Glib::ustring::compose(
        Glib::ustring(gettext("Variable %1 is recursively referenced.\nModify the expression and try again.")),
        Glib::ustring(key));
    beep();
    Gtk::MessageDialog dialog(*this,
                              message,
                              false,
                              Gtk::MESSAGE_WARNING);
    dialog.set_title(appDisplayName);
    dialog.run();
}


void MainWindow::pasteFromHistory(size_t index)
{
    if (index < history.size())
    {
        const Glib::ustring& s = history[index];
        input = s.c_str();
    }
}


void MainWindow::onHistoryChange()
{
    uiManager->get_widget("/MenuBar/Edit/Back")->set_sensitive(history.getIndex() ? true : false);
    uiManager->get_widget("/MenuBar/Edit/Next")->set_sensitive(history.getIndex() + 1 < history.size() ? true : false);
    uiManager->get_widget("/ToolBar/Back")->set_sensitive(history.getIndex() ? true : false);
    uiManager->get_widget("/ToolBar/Next")->set_sensitive(history.getIndex() + 1 < history.size() ? true : false);
}


void MainWindow::onVariableDialogResponse(int response)
{
    switch (response)
    {
    case RESPONSE_COPY:
    {
        const Glib::ustring& key = variableDialog.getSelected();
        if (!key.empty() && key.length() == 1) // only A to Z can be copied; others are treated as read-only.
        {
            VariableStore::instance().setValue(key, Glib::ustring(input, input.size()));
        }
        break;
    }
    case RESPONSE_PASTE:
    {
        const Glib::ustring& key = variableDialog.getSelected();
        if (!key.empty())
        {
            input.putString(VariableStore::instance().getValue(key).c_str());
        }
        break;
    }
    default:
        variableDialog.hide();
        uiManager->get_widget("/MenuBar/Edit/Variables")->set_sensitive(true);
        uiManager->get_widget("/ToolBar/Variables")->set_sensitive(true);
        break;
    }
}
