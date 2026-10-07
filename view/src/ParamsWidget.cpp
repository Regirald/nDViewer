
#define QT_NO_DEPRECATED_WARNINGS
#include "ParamsWidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

ParamsWidget::ParamsWidget(QWidget* parent)
	: QWidget(parent)
{
	// Create viewer and keep polytope alive in a shared_ptr
	m_viewer = new Viewer(this);
	m_viewer->setPolytope(m_currentPolytope.get());

	// Controls
	m_spinA = new QSpinBox();
	m_spinA->setMinimum(2);
	m_spinA->setMaximum(20);
	m_spinA->setValue(3);

	m_spinB = new QSpinBox();
	m_spinB->setMinimum(2);
	m_spinB->setMaximum(20);
	m_spinB->setValue(3);

	m_spinC = new QSpinBox();
	m_spinC->setMinimum(2);
	m_spinC->setMaximum(20);
	m_spinC->setValue(3);

	m_generateBtn = new QPushButton("Generate");
	m_shadedCheck = new QCheckBox("Full shaded rendering");
	m_shadedCheck->setChecked(true);

	// Layout
	auto* hLayout = new QHBoxLayout(this);
	auto* controls = new QWidget();
	auto* vLayout = new QVBoxLayout();

	vLayout->addWidget(new QLabel("Schlafli [a, b, c]"));
	vLayout->addWidget(m_spinA);
	vLayout->addWidget(m_spinB);
	vLayout->addWidget(m_spinC);
	vLayout->addWidget(m_shadedCheck);
	vLayout->addWidget(m_generateBtn);
	vLayout->addStretch();

	controls->setLayout(vLayout);

	hLayout->addWidget(m_viewer, 1);
	hLayout->addWidget(controls);

	// Connections
	connect(m_generateBtn, &QPushButton::clicked, this, [this]() {
		int a = m_spinA->value();
		int b = m_spinB->value();
		int c = m_spinC->value();
		m_currentPolytope = std::make_shared<PolytopeGeometry3D>(RegularPolytopeGenerator<4>::generateRegularPolytope({ a, b, c }));
		m_viewer->setPolytope(m_currentPolytope.get());
		m_viewer->update();
	});

	connect(m_shadedCheck, &QCheckBox::toggled, this, [this](bool checked) {
		m_viewer->setFullShaded(checked);
		m_viewer->update();
	});
}
