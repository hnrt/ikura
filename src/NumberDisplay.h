// Copyright (C) 2014-2017 Hideaki Narita


#ifndef IKURA_NUMBERDISPLAY_H
#define IKURA_NUMBERDISPLAY_H


#include <vector>
#include <gtkmm.h>


namespace hnrt
{
    class NumberDisplay : public Gtk::ScrolledWindow
    {
    public:

        NumberDisplay(const char* text = "")
            : contents(text)
            , borderLineWidth(0)
        {
            set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_NEVER); // only horizontal scrollbar is used.
            add(box);
            contents.set_alignment(1.0, 0.5); // h=right, v=center
            box.add(contents);
            contents.signal_realize().connect(sigc::mem_fun(*this, &NumberDisplay::onContentsRealize));
            get_hadjustment()->signal_changed().connect(sigc::mem_fun(*this, &NumberDisplay::onAdjustmentChange));
        }
        virtual ~NumberDisplay() {}
        void set_padding(int x, int y) { contents.set_padding(x, y); }
        void set_text(const Glib::ustring& s) { contents.set_text(s); }
        void modify_font(const Pango::FontDescription& fd) { contents.modify_font(fd); }
        void modify_bg(Gtk::StateType state, const Gdk::Color& color) { box.modify_bg(state, color); }
        double getAdjustmentLower() const { return 0.0; }
        double getAdjustmentUpper() const { return getContentsWidth() - getViewportWidth(); }
        double getAdjustmentStep() const { return get_hadjustment()->get_step_increment(); }
        double getAdjustmentValue() const { return get_hadjustment()->get_value(); }
        void setAdjustmentValue(double value) { get_hadjustment()->set_value(value); }

    protected:

        NumberDisplay(const NumberDisplay&) {}
        int getViewportWidth() const { return get_child()->get_allocation().get_width() - borderLineWidth; }
        int getContentsWidth() const { return contents.get_allocation().get_width(); }
        void onContentsRealize()
        {
            Gtk::Allocation a1 = get_child()->get_allocation();
            Gtk::Allocation a2 = contents.get_allocation();
            borderLineWidth = a1.get_width() - a2.get_width();
        }
        void onAdjustmentChange()
        {
            setAdjustmentValue(getAdjustmentUpper());
        }

        //
        // Label is wrapped by EventBox to change its background color.
        //
        Gtk::EventBox box;
        Gtk::Label contents;
        int borderLineWidth; // total width of the left and right viewport border line
    };
}


#endif //!IKURA_NUMBERDISPLAY_H
