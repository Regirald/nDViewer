#pragma once

#include <QMainWindow>
#include "Viewer.h"
#include "ParamsWidget.h"
#include "model/GeometryFromTopology.h"
#include "logic/GenerateRegularPolytope.h"
#include <memory>
#include <QHBoxLayout>


class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	explicit MainWindow(QWidget* parent = nullptr);

	Viewer* getViewer() const { return m_viewer; }
	ParamsWidget* getParamsWidget() const { return m_paramsWidget; }

public:
	void onGenerate(int a, int b, int c)
	{
		PolytopeGeometry3D poly = RegularPolytopeGenerator<4>::generateRegularPolytope({ a, b, c });
		m_currentPolytope = std::make_shared<PolytopeGeometry3D>(poly);
		m_viewer->setPolytope(m_currentPolytope.get());
		m_viewer->update();
	}

	void onShadedToggled(bool enabled)
	{
		m_viewer->setFullShaded(enabled);
		m_viewer->update();
	}

private:
	ParamsWidget* m_paramsWidget = nullptr;
	Viewer* m_viewer = nullptr;
	std::shared_ptr<PolytopeGeometry3D> m_currentPolytope;
};

// Inline implementation for convenience
inline MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
{
	// central widget and layout
	QWidget* central = new QWidget(this);
	auto* hLayout = new QHBoxLayout(central);

	m_viewer = new Viewer(central);
	m_paramsWidget = new ParamsWidget(central);

	hLayout->addWidget(m_viewer, 1);
	hLayout->addWidget(m_paramsWidget);
	central->setLayout(hLayout);
	setCentralWidget(central);

	// initialize polytope storage and viewer
	m_currentPolytope = std::make_shared<PolytopeGeometry3D>();
	m_viewer->setPolytope(m_currentPolytope.get());

	// connections: params -> main window slots (connect widget signals directly)
	connect(m_paramsWidget->generateButton(), &QPushButton::clicked, this, [this]() {
		onGenerate(m_paramsWidget->valueA(), m_paramsWidget->valueB(), m_paramsWidget->valueC());
	});
	connect(m_paramsWidget->shadedCheckBox(), &QCheckBox::toggled, this, &MainWindow::onShadedToggled);
}
