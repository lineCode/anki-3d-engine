// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Sslf.h"
#include "anki/renderer/Renderer.h"
#include "anki/renderer/Pps.h"
#include "anki/renderer/Bloom.h"
#include "anki/misc/ConfigSet.h"

namespace anki {

//==============================================================================
Error Sslf::init(const ConfigSet& config)
{
	Error err = initInternal(config);
	if(err)
	{
		ANKI_LOGE("Failed to init screen space lens flare pass");
	}

	return err;
}

//==============================================================================
Error Sslf::initInternal(const ConfigSet& config)
{
	m_enabled = config.getNumber("pps.sslf.enabled");
	if(!m_enabled)
	{
		return ErrorCode::NONE;
	}

	const PixelFormat pixelFormat(
		ComponentFormat::R8G8B8, TransformFormat::UNORM);

	// Load program 1
	StringAuto pps(getAllocator());

	pps.sprintf(
		"#define TEX_DIMENSIONS vec2(%u.0, %u.0)\n",
		m_r->getPps().getBloom().getWidth(),
		m_r->getPps().getBloom().getHeight());

	ANKI_CHECK(m_frag.loadToCache(&getResourceManager(),
		"shaders/PpsSslf.frag.glsl", pps.toCString(), "r_"));

	ColorStateInfo colorState;
	colorState.m_attachmentCount = 1;
	colorState.m_attachments[0].m_format = pixelFormat;

	m_r->createDrawQuadPipeline(m_frag->getGrShader(), colorState, m_ppline);

	// Textures
	ANKI_CHECK(m_lensDirtTex.load(
		"engine_data/lens_dirt.ankitex", &getResourceManager()));

	// Create the render target and FB
	m_r->createRenderTarget(m_r->getPps().getBloom().getWidth(),
		m_r->getPps().getBloom().getHeight(),
		pixelFormat, 1, SamplingFilter::LINEAR, 1, m_rt);

	FramebufferInitializer fbInit;
	fbInit.m_colorAttachmentsCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation =
		AttachmentLoadOperation::DONT_CARE;
	m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

	// Create the resource group
	ResourceGroupInitializer rcInit;
	rcInit.m_textures[0].m_texture = m_r->getPps().getBloom().getRt();
	rcInit.m_textures[1].m_texture = m_lensDirtTex->getGlTexture();

	m_rcGroup = getGrManager().newInstance<ResourceGroup>(rcInit);

	return ErrorCode::NONE;
}

//==============================================================================
void Sslf::run(CommandBufferPtr& cmdb)
{
	ANKI_ASSERT(m_enabled);

	// Draw to the SSLF FB
	cmdb->bindFramebuffer(m_fb);
	cmdb->setViewport(0, 0, m_r->getPps().getBloom().getWidth(),
		m_r->getPps().getBloom().getHeight());

	cmdb->bindPipeline(m_ppline);
	cmdb->bindResourceGroup(m_rcGroup);

	m_r->drawQuad(cmdb);
}

} // end namespace anki
