#pragma once

#include <QMainWindow>
#include "Viewer.h"
#include "ParamsWidget.h"
#include "model/GeometryFromTopology.h"

class MainWindow : public QMainWindow
{
public:
	explicit MainWindow(QWidget* parent = nullptr);

	Viewer* getViewer() const { return m_viewer; }
	ParamsWidget* getParamsWidget() const { return m_paramsWidget; }

private:
	ParamsWidget* m_paramsWidget = nullptr;
	Viewer* m_viewer = nullptr;
};

// Inline implementation to ensure the symbol is available to callers that include this header.
inline MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
{
	m_paramsWidget = new ParamsWidget(this);
	setCentralWidget(m_paramsWidget);
	m_viewer = m_paramsWidget->viewer();
}
