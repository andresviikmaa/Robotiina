#pragma once
#include "types.h"
#include <functional>
#include <boost/thread/mutex.hpp>

/*
* No time to add QT support to OpenCV, have to make our own buttons (and dialogs)
* */

class Dialog {
public:
    Dialog(const std::string &m_Title, int flags = CV_WINDOW_AUTOSIZE);
	int createButton(const std::string& bar_name, std::function<void()> const &);
    int show(const cv::Mat background);
	void clearButtons() {
		m_buttons.clear();
	}
protected:
    void mouseClicked(int x, int y);
private:
    bool m_close = false;
    std::string m_title;
	std::vector<std::tuple<std::string, std::function<void()>>> m_buttons;
    int m_buttonHeight = 60; /* calculated automatically*/
	boost::mutex mutex;

};
