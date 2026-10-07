
#define QT_NO_DEPRECATED_WARNINGS
#include "ParamsWidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

ParamsWidget::ParamsWidget(QWidget* parent)
	: QWidget(parent)
{
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

	hLayout->addWidget(controls);

	// No direct connections to the viewer here; MainWindow will connect to the buttons/signals
}

int ParamsWidget::valueA() const { return m_spinA->value(); }
int ParamsWidget::valueB() const { return m_spinB->value(); }
int ParamsWidget::valueC() const { return m_spinC->value(); }
bool ParamsWidget::shaded() const { return m_shadedCheck->isChecked(); }
