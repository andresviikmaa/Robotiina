#pragma once
#include "types.h"
/*
* No time to add QT support to OpenCV, have to make our own buttons (and dialogs)
* */
class Dialog {
public:
    Dialog(const std::string &m_Title, int flags = CV_WINDOW_AUTOSIZE);
    int createButton( const std::string& bar_name, cv::ButtonCallback on_change,
            void* userdata=NULL, int type=CV_PUSH_BUTTON,
            bool initial_button_state=0);
    int show();
protected:
    void mouseClicked(int x, int y);
private:
    bool m_close = false;
    std::string m_title;
    std::vector<std::tuple<std::string, cv::ButtonCallback, void*>> m_buttons;
    int m_buttonHeight = 60; /* calculated automatically*/

};
