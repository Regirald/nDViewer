#include <QCheckBox>
#include <QHBoxLayout>
#include <QPushButton>

#include "MainWindow.h"
#include "model/GeometryFromTopology.h"
#include "logic/GenerateRegularPolytope.h"

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
{
	QWidget* central = new QWidget(this);
	auto* hLayout = new QHBoxLayout(central);

	m_viewer = new Viewer(central);
	m_paramsWidget = new ParamsWidget(central);

	hLayout->addWidget(m_viewer, 1);
	hLayout->addWidget(m_paramsWidget);
	central->setLayout(hLayout);
	setCentralWidget(central);

	m_currentPolytope = std::make_unique<AbstractPolytopeGeometry>();
	m_viewer->setPolytope(m_currentPolytope.get());

	connect(m_paramsWidget->generateButton(), &QPushButton::clicked, this, [this]() {
		onGenerate(m_paramsWidget->valueA(), m_paramsWidget->valueB(), m_paramsWidget->valueC());
		});
	connect(m_paramsWidget->shadedCheckBox(), &QCheckBox::toggled, this, &MainWindow::onShadedToggled);
}

void MainWindow::onGenerate(int a, int b, int c)
{
	std::vector<int> schlafli = { a, b, c };
	m_currentPolytope.reset(RegularPolytopeGenerator::generateRegularPolytope(schlafli));
	m_viewer->setPolytope(m_currentPolytope.get());
	m_viewer->update();
}

void MainWindow::onShadedToggled(bool enabled)
{
	m_viewer->setFullShaded(enabled);
	m_viewer->update();
}
