#include "VerticesRenderer.h"
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>

#include "model/Sphere.h"


void VerticesRenderer::initializeGLResources()
{
	if (!m_poly)
		return;

	// build sphere data from polytope vertices
	std::vector<SpheresCollection::Point> centers;
	std::vector<float> radii;
	centers.reserve(m_poly->vertices.size());
	radii.reserve(m_poly->vertices.size());

	std::vector<Sphere> sphereList; // Store pointers to individual spheres
	for (const auto& v : m_poly->vertices)
	{
		centers.push_back(v);
		radii.push_back(1.f); // default sphere radius
		sphereList.push_back(Sphere(v, 1.f));
	}
	std::shared_ptr<ImplicitFunction<3>> sphereFinal = std::make_shared<Sphere>(sphereList[0]);
	for (int i = 1; i < sphereList.size(); ++i)
	{
		sphereFinal = sphereFinal + std::static_pointer_cast<ImplicitFunction<3>>(std::make_shared<Sphere>(sphereList[i]));
	}

	m_spheres = std::make_unique<SpheresCollection>(centers, radii);

	//Sphere* singleSphere = new Sphere(centers[0], radii[0]);
	//m_renderer = std::make_unique<RenderImplicitFunction<3>>(singleSphere);
	m_renderer = std::make_unique<RenderImplicitFunction<3>>(sphereFinal);
	m_renderer->initializeGLResources();
}

void VerticesRenderer::render(const ndv::RenderContext& context)
{
	if (!m_renderer)
		return;
	m_renderer->render(context);
}
