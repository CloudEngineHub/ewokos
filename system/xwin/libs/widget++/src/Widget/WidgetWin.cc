#include <Widget/WidgetWin.h>
#include <ewoksys/timer.h>

namespace Ewok {

WidgetWin::WidgetWin() {
	root = NULL;
	timerID = 0;
	painting = false;
	timerFPS = 1;
	timerStep = 0;
}

WidgetWin::~WidgetWin() {
	if(root != NULL)
		delete root;
	if(timerID > 0)
		timer_remove(timerID);
}

void WidgetWin::onRepaint(graph_t* g) {
	if(root == NULL)
		return;

	painting = true;
	root->repaint(g, &theme);
	painting = false;
}

void WidgetWin::onShow(void) {
	if(root)
		root->update();
}

void WidgetWin::onFocus(void) {
	if(root == NULL)
		return;
	root->onFocus();
}

void WidgetWin::onResize(void) {
	if(root == NULL)
		return;
	root->resizeTo(xwin->xinfo->wsr.w, xwin->xinfo->wsr.h);
}	

void WidgetWin::onEvent(xevent_t* ev) {
	if(root == NULL)
		return;
	if(root->sendEvent(ev))
		root->repaintWin();
}

bool WidgetWin::onClose() {
	if(timerID > 0)
		timer_remove(timerID);
	return true;
}

void WidgetWin::timerTask() {
	if(root == NULL)
		return;

	root->timerTrigger(timerFPS, timerStep++);
}

void WidgetWin::doTimer() {
	if(root == NULL)
		return;

	root->doTimer();
}

void WidgetWin::setRoot(RootWidget* root) {
	if(this->root != NULL)
		delete this->root;
	root->xwin = this;
	this->root = root;
	setAlpha(root->isAlpha());
}

void WidgetWin::build() {
	onBuild();
}

void WidgetWin::onBuild() {
	RootWidget* r = new RootWidget();
	r->setType(Container::VERTICLE);
	r->setAlpha(false);
	setRoot(r);
}

static WidgetWin* _win = NULL;

static void _timerHandler(void) {
	_win->timerTask();
}

void WidgetWin::setTimer(uint32_t fps) {
	if(fps < TIMER_MIN_FPS)
		fps = TIMER_MIN_FPS;
	timerFPS = fps;

	_win = this;
	if(timerID > 0)
		timer_remove(timerID);
	timerID = timer_set(1000*1000/timerFPS, _timerHandler);
}

}

