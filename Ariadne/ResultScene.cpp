#include "ResultScene.h"

namespace Ariadne {

ResultScene::ResultScene() : font(20) {}

void ResultScene::init() {
	Graphics::SetBackground(BACKGROUND_COLOR);

	auto& scores = m_data->scores[m_data->levelName];
	scores.emplace_back(m_data->elapsed);
	std::sort(scores.begin(), scores.end());

	CSVWriter csv(SCORES_CSV);
	for (const auto& p : m_data->scores) {
		csv.write(p.first);
		for (const auto& m : p.second) {
			csv.write(m.count());
		}
		csv.nextLine();
	}
	csv.close();
}

void ResultScene::update() {
	if (Input::KeyEnter.clicked || Input::MouseL.clicked) {
		changeScene(SceneType::TITLE);
	}
}

void ResultScene::draw() const {
	const auto& scores = m_data->scores[m_data->levelName];
	const auto lowerBound = std::find(scores.begin(), scores.end(), m_data->elapsed);
	if (!scores.empty()) {
		for (auto it = scores.begin(); it != scores.end(); ++it) {
			const auto rank = std::distance(scores.begin(), it) + 1;
			font(format(rank, *it)).drawCenter(Window::Center() + Point(0, 50 * (it - lowerBound)));
		}
	}
	const auto rank = std::distance(scores.begin(), lowerBound) + 1;
	font(format(rank, m_data->elapsed)).drawCenter({Window::Width() / 2, Window::Height() / 2}, Palette::Red);
}

String ResultScene::format(const size_t rank, const Milliseconds& ms) const {
	const auto min = ms.count() / (60 * 1000);
	const auto sec = (ms.count() % (60 * 1000)) / 1000;
	const auto subSec = (ms.count() % 1000) / 10 + ((ms.count() % 10 >= 5) ? 1 : 0);
	return Format(PyFmtString(m_data->textAssets.at(L"ranking_format")),
				  rank, Pad(min, {2, '0'}), Pad(sec, {2, '0'}), Pad(subSec, {2, '0'}));
}

}
