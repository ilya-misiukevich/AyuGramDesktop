#include "ayu/ui/boxes/smooth_scroll.h"

#include "ui/effects/animation_value.h"
#include "ui/effects/animations.h"
#include "ui/widgets/scroll_area.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QScrollBar>

namespace AyuUi {
namespace {

constexpr auto kScrollStepTime = crl::time(260);

} // namespace

void SetupSmoothWheelScroll(not_null<Ui::ScrollArea*> scroll) {
	struct State {
		Ui::Animations::Basic animation;
		int fromTop = 0;
		int targetTop = 0;
		crl::time startTime = 0;
	};
	const auto state = scroll->lifetime().make_state<State>();
	const auto single = scroll->verticalScrollBar()->singleStep()
		* QApplication::wheelScrollLines();
	// WHY: a wheel notch jumps a whole step at once; the profile animates
	// it in info_flexible_scroll.cpp, copied here without its cover-specific
	// snap steps.
	state->animation.init([=](crl::time now) {
		const auto progress = std::clamp(
			(now - state->startTime) / float64(kScrollStepTime),
			0.,
			1.);
		scroll->scrollToY(anim::interpolate(
			state->fromTop,
			state->targetTop,
			anim::easeOutQuint(1., progress)));
		if (progress >= 1.) {
			state->animation.stop();
		}
	});
	scroll->setCustomWheelProcess([=](not_null<QWheelEvent*> e) {
		const auto delta = e->angleDelta().y();
		if (std::abs(delta) != 120 || e->phase() != Qt::NoScrollPhase) {
			state->animation.stop();
			return false;
		}
		const auto base = state->animation.animating()
			? state->targetTop
			: scroll->scrollTop();
		const auto diff = (delta > 0) ? -single : single;
		state->targetTop = std::clamp(
			base + diff,
			0,
			scroll->scrollTopMax());
		state->fromTop = scroll->scrollTop();
		state->startTime = crl::now();
		if (!state->animation.animating()) {
			state->animation.start();
		}
		return true;
	});
}

} // namespace AyuUi
