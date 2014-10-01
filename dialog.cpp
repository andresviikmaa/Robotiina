#include "dialog.h"

Dialog::Dialog(const std::string &title, int flags/* = CV_WINDOW_AUTOSIZE*/) {

    m_title = title;
    int baseLine;
    m_buttonHeight = cv::getTextSize("Ajig6", cv::FONT_HERSHEY_DUPLEX, 1.5, 1, &baseLine).height * 2;
};

int Dialog::createButton( const std::string& bar_name, cv::ButtonCallback on_change,
        void* userdata/*=NULL*/, int type/*=CV_PUSH_BUTTON*/,
        bool initial_button_state/*=0*/){

    m_buttons.push_back(std::make_tuple(bar_name, on_change, userdata));
};

int Dialog::show() {

    int window_width = 1024;
    int window_height = 760;
    cv::Mat image = cv::Mat::zeros( window_height, window_width, CV_8UC3 );

    int i = 0;
    for (const auto& button : m_buttons) {
        cv::putText(image, std::get<0>(button), cv::Point(30, (++i)*m_buttonHeight ), cv::FONT_HERSHEY_DUPLEX, 1.5, cv::Scalar(255,255,255));

    }
    cv::namedWindow(m_title, CV_WINDOW_FULLSCREEN);
    cv::imshow( m_title, image );

    cv::setMouseCallback(m_title, [](int event, int x, int y, int flags, void* self) {
            if (event==cv::EVENT_LBUTTONUP){
                ((Dialog*)self)->mouseClicked(x, y);
            }
    }, this);
    m_close = false;
    while(!m_close){
        if (cv::waitKey(30) == 27) {
           break;
        }
    }
    cv::destroyWindow(m_title);
    return 0;
};

void Dialog::mouseClicked(int x, int y) {
    int index = (y / m_buttonHeight );
    if (index < m_buttons.size()){
        auto button = m_buttons[index];
        std::get<1>(button)(1, std::get<2>(button));
        m_close = true;
    }

}