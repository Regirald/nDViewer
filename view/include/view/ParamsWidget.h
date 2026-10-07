#pragma once

#include <QWidget>
#include <memory>

#include <QSpinBox>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>

#include "Viewer.h"
#include "model/GeometryFromTopology.h"
#include "logic/GenerateRegularPolytope.h"

class ParamsWidget : public QWidget
{
public:
	// Construct with an initial polytope to display
	ParamsWidget(QWidget* parent = nullptr);

	// Access the contained Viewer instance (owned by this widget)
	Viewer* viewer() const { return m_viewer; }

private:
	Viewer* m_viewer = nullptr;
	std::shared_ptr<PolytopeGeometry3D> m_currentPolytope;

	QSpinBox* m_spinA = nullptr;
	QSpinBox* m_spinB = nullptr;
	QSpinBox* m_spinC = nullptr;
	QPushButton* m_generateBtn = nullptr;
	QCheckBox* m_shadedCheck = nullptr;
};
