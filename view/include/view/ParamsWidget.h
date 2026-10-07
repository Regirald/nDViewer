#pragma once

#include <QWidget>
#include <memory>

#include <QSpinBox>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>

#include "model/GeometryFromTopology.h"
#include "logic/GenerateRegularPolytope.h"

class ParamsWidget : public QWidget
{
	Q_OBJECT

public:
	// Construct with an initial polytope to display
	explicit ParamsWidget(QWidget* parent = nullptr);

	// Accessors used by MainWindow to read widget state and connect signals
	int valueA() const;
	int valueB() const;
	int valueC() const;
	bool shaded() const;

public slots:
	QPushButton* generateButton() const { return m_generateBtn; }
	QCheckBox* shadedCheckBox() const { return m_shadedCheck; }

private:

	QSpinBox* m_spinA = nullptr;
	QSpinBox* m_spinB = nullptr;
	QSpinBox* m_spinC = nullptr;
	QPushButton* m_generateBtn = nullptr;
	QCheckBox* m_shadedCheck = nullptr;
};
// End of ParamsWidget.h
