// Copyright (C) 2014-2017 Hideaki Narita


#include <libintl.h>
#include "LocaleInfo.h"
#include "MainWindow.h"


#define TEXTDOMAIN "ikura"


using namespace hnrt;


int main(int argc, char *argv[])
{
    LocaleInfo::instance().init(); // initialization for internationalization

    // initialization for UI localization
    bindtextdomain(TEXTDOMAIN, LocaleInfo::instance().getMessageCatalogDir(TEXTDOMAIN).c_str());
    bind_textdomain_codeset(TEXTDOMAIN, "UTF-8");
    textdomain(TEXTDOMAIN);

    // main application logic
    Gtk::Main kit(argc, argv);
    MainWindow window;
    Gtk::Main::run(window);

    return 0;
}
