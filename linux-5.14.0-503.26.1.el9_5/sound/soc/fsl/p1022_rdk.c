// SPDX-License-Identifier: GPL-2.0
//
// Freescale P1022RDK ALSA SoC Machine driver
//
// Author: Timur Tabi <timur@freescale.com>
//
// Copyright 2012 Freescale Semiconductor, Inc.
//
// Note: in order for audio to work correctly, the output controls need
// to be enabled, because they control the clock.  So for playback, for
// example:
//
//      amixer sset 'Left Output Mixer PCM' on
//      amixer sset 'Right Output Mixer PCM' on

#include <linux/module.h>
#include <linux/fsl/guts.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/slab.h>
#include <sound/soc.h>

#include "fsl_dma.h"
#include "fsl_ssi.h"
#include "fsl_utils.h"

/* P1022-specific PMUXCR and DMUXCR bit definitions */

#define CCSR_GUTS_PMUXCR_UART0_I2C1_MASK	0x0001c000
#define CCSR_GUTS_PMUXCR_UART0_I2C1_UART0_SSI	0x00010000
#define CCSR_GUTS_PMUXCR_UART0_I2C1_SSI		0x00018000

#define CCSR_GUTS_PMUXCR_SSI_DMA_TDM_MASK	0x00000c00
#define CCSR_GUTS_PMUXCR_SSI_DMA_TDM_SSI	0x00000000

#define CCSR_GUTS_DMUXCR_PAD	1	/* DMA controller/channel set to pad */
#define CCSR_GUTS_DMUXCR_SSI	2	/* DMA controller/channel set to SSI */

/*
 * Set the DMACR register in the GUTS
 *
 * The DMACR register determines the source of initiated transfers for each
 * channel on each DMA controller.  Rather than have a bunch of repetitive
 * macros for the bit patterns, we just have a function that calculates
 * them.
 *
 * guts: Pointer to GUTS structure
 * co: The DMA controller (0 or 1)
 * ch: The channel on the DMA controller (0, 1, 2, or 3)
 * device: The device to set as the target (CCSR_GUTS_DMUXCR_xxx)
 */
static inline void guts_set_dmuxcr(struct ccsr_guts __iomem *guts,
	unsigned int co, unsigned int ch, unsigned int device)
{
	unsigned int shift = 16 + (8 * (1 - co) + 2 * (3 - ch));

	clrsetbits_be32(&guts->dmuxcr, 3 << shift, device << shift);
}

/* There's only one global utilities register */
static phys_addr_t guts_phys;

/*
 * machine_data: machine-specific ASoC device data
 *
 * This structure contains data for a single sound platform device on an
 * P1022 RDK.  Some of the data is taken from the device tree.
 */
struct machine_data {
	struct snd_soc_dai_link dai[2];
	struct snd_soc_card card;
	unsigned int dai_format;
	unsigned int codec_clk_direction;
	unsigned int cpu_clk_direction;
	unsigned int clk_frequency;
	unsigned int dma_id[2];		/* 0 = DMA1, 1 = DMA2, etc */
	unsigned int dma_channel_id[2]; /* 0 = ch 0, 1 = ch 1, etc*/
	char platform_name[2][DAI_NAME_SIZE]; /* One for each DMA channel */
};

/**
 * p1022_rdk_machine_probe - initialize the board
 * @card: ASoC card instance
 *
 * This function is used to initialize the board-specific hardware.
 *
 * Here we program the DMACR and PMUXCR registers.
 *
 * Returns: %0 on success or negative errno value on error
 */
static int p1022_rdk_machine_probe(struct snd_soc_card *card)
{
	struct machine_data *mdata =
		container_of(card, struct machine_data, card);
	struct ccsr_guts __iomem *guts;

	guts = ioremap(guts_phys, sizeof(struct ccsr_guts));
	if (!guts) {
		dev_err(card->dev, "could not map global utilities\n");
		return -ENOMEM;
	}

	/* Enable SSI Tx signal */
	clrsetbits_be32(&guts->pmuxcr, CCSR_GUTS_PMUXCR_UART0_I2C1_MASK,
			CCSR_GUTS_PMUXCR_UART0_I2C1_UART0_SSI);

	/* Enable SSI Rx signal */
	clrsetbits_be32(&guts->pmuxcr, CCSR_GUTS_PMUXCR_SSI_DMA_TDM_MASK,
			CCSR_GUTS_PMUXCR_SSI_DMA_TDM_SSI);

	/* Enable DMA Channel for SSI */
	guts_set_dmuxcr(guts, mdata->dma_id[0], mdata->dma_channel_id[0],
			CCSR_GUTS_DMUXCR_SSI);

	guts_set_dmuxcr(guts, mdata->dma_id[1], mdata->dma_channel_id[1],
			CCSR_GUTS_DMUXCR_SSI);

	iounmap(guts);

	return 0;
}

/**
 * p1022_rdk_startup - program the board with various hardware parameters
 * @substream: ASoC substream object
 *
 * This function takes board-specific information, like clock frequencies
 * and serial data formats, and passes that information to the codec and
 * transport drivers.
 *
 * Returns: %0 on success or negative errno value on error
 */
static int p1022_rdk_startup(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = snd_soc_substream_to_rtd(substream);
	struct machine_data *mdata =
		container_of(rtd->card, struct machine_data, card);
	struct device *dev = rtd->card->dev;
	int ret = 0;

	/* Tell the codec driver what the serial protocol is. */
	ret = snd_soc_dai_set_fmt(snd_soc_rtd_to_codec(rtd, 0), mdata->dai_format);
	if (ret < 0) {
		dev_err(dev, "could not set codec driver audio format (ret=%i)\n",
			ret);
		return ret;
	}

	ret = snd_soc_dai_set_pll(snd_soc_rtd_to_codec(rtd, 0), 0, 0, mdata->clk_frequency,
		mdata->clk_frequency);
	if (ret < 0) {
		dev_err(dev, "could not set codec PLL frequency (ret=%i)\n",
			ret);
		return ret;
	}

	return 0;
}

/**
 * p1022_rdk_machine_remove - Remove the sound device
 * @card: ASoC card instance
 *
 * This function is called to remove the sound device for one SSI.  We
 * de-program the DMACR and PMUXCR register.
 *
 * Returns: %0 on success or negative errno value on error
 */
static int p1022_rdk_machine_remove(struct snd_soc_card *card)
{
	struct machine_data *mdata =
		container_of(card, struct machine_data, card);
	struct ccsr_guts __iomem *guts;

	guts = ioremap(guts_phys, sizeof(struct ccsr_guts));
	if (!guts) {
		dev_err(card->dev, "could not map global utilities\n");
		return -ENOMEM;
	}

	/* Restore the signal routing */
	clrbits32(&guts->pmuxcr, CCSR_GUTS_PMUXCR_UART0_I2C1_MASK);
	clrbits32(&guts->pmuxcr, CCSR_GUTS_PMUXCR_SSI_DMA_TDM_MASK);
	guts_set_dmuxcr(guts, mdata->dma_id[0], mdata->dma_channel_id[0], 0);
	guts_set_dmuxcr(guts, mdata->dma_id[1], mdata->dma_channel_id[1], 0);

	iounmap(guts);

	return 0;
}

/*
 * p1022_rdk_ops: ASoC machine driver operations
 */
static const struct snd_soc_ops p1022_rdk_ops = {
	.startup = p1022_rdk_startup,
};

/**
 * p1022_rdk_probe - platform probe function for the machine driver
 * @pdev: platform device pointer
 *
 * Although this is a machine driver, the SSI node is the "master" node with
 * respect to audio hardware connections.  Therefore, we create a new ASoC
 * device for each new SSI node that has a codec attached.
 *
 * Returns: %0 on success or negative errno value on error
 */
static int p1022_rdk_probe(struct platform_device *pdev)
{
	struct device *dev = pdev->dev.parent;
	/* ssi_pdev is the platform device for the SSI node that probed us */
	struct platform_device *ssi_pdev = to_platform_device(dev);
	struct device_node *np = ssi_pdev->dev.of_node;
	struct device_node *codec_np = NULL;
	struct machine_data *mdata;
	struct snd_soc_dai_link_component *comp;
	const u32 *iprop;
	int ret;

	/* Find the codec node for this SSI. */
	codec_np = of_parse_phandle(np, "codec-handle", 0);
	if (!codec_np) {
		dev_err(dev, "could not find codec node\n");
		return -EINVAL;
	}

	mdata = kzalloc(sizeof(struct machine_data), GFP_KERNEL);
	if (!mdata) {
		ret = -ENOMEM;
		goto error_put;
	}

	comp = devm_kzalloc(&pdev->dev, 6 * sizeof(*comp), GFP_KERNEL);
	if (!comp) {
		ret = -ENOMEM;
		goto error_put;
	}

	mdata->dai[0].cpus	= &comp[0];
	mdata->dai[0].codecs	= &comp[1];
	mdata->dai[0].platforms	= &comp[2];

	mdata->dai[0].num_cpus		= 1;
	mdata->dai[0].num_codecs	= 1;
	mdata->dai[0].num_platforms	= 1;

	mdata->dai[1].cpus	= &comp[3];
	mdata->dai[1].codecs	= &comp[4];
	mdata->dai[1].platforms	= &comp[5];

	mdata->dai[1].num_cpus		= 1;
	mdata->dai[1].num_codecs	= 1;
	mdata->dai[1].num_platforms	= 1;

	mdata->dai[0].cpus->dai_name = dev_name(&ssi_pdev->dev);
	mdata->dai[0].ops = &p1022_rdk_ops;

	/* ASoC core can match codec with device node */
	mdata->dai[0].codecs->of_node = codec_np;

	/*
	 * We register two DAIs per SSI, one for playback and the other for
	 * capture.  We support codecs that have separate DAIs for both playback
	 * and capture.
	 */
	memcpy(&mdata->dai[1], &mdata->dai[0], sizeof(struct snd_soc_dai_link));

	/* The DAI names from the codec (snd_soc_dai_driver.name) */
	mdata->dai[0].codecs->dai_name = "wm8960-hifi";
	mdata->dai[1].codecs->dai_name = mdata->dai[0].codecs->dai_name;

	/*
	 * Configure the SSI for I2S slave mode.  Older device trees have
	 * an fsl,mode property, but we ignore that since there's really
	 * only one way to configure the SSI.
	 */
	mdata->dai_format = SND_SOC_DAIFMT_NB_NF |
		SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CBP_CFP;
	mdata->codec_clk_direction = SND_SOC_CLOCK_OUT;
	mdata->cpu_clk_direction = SND_SOC_CLOCK_IN;

	/*
	 * In i2s-slave mode, the codec has its own clock source, so we
	 * need to get the frequency from the device tree and pass it to
	 * the codec driver.
	 */
	iprop = of_get_property(codec_np, "clock-frequency", NULL);
	if (!iprop || !*iprop) {
		dev_err(&pdev->dev, "codec bus-frequency property is missing or invalid\n");
		ret = -EINVAL;
		goto error;
	}
	mdata->clk_frequency = be32_to_cpup(iprop);

	if (!mdata->clk_frequency) {
		dev_err(&pdev->dev, "unknown clock frequency\n");
		ret = -EINVAL;
		goto error;
	}

	/* Find the playback DMA channel to use. */
	mdata->dai[0].platforms->name = mdata->platform_name[0];
	ret = fsl_asoc_get_dma_channel(np, "fsl,playback-dma", &mdata->dai[0],
				       &mdata->dma_channel_id[0],
				       &mdata->dma_id[0]);
	if (ret) {
		dev_err(&pdev->dev, "missing/invalid playback DMA phandle (ret=%i)\n",
			ret);
		goto error;
	}

	/* Find the capture DMA channel to use. */
	mdata->dai[1].platforms->name = mdata->platform_name[1];
	ret = fsl_asoc_get_dma_channel(np, "fsl,capture-dma", &mdata->dai[1],
				       &mdata->dma_channel_id[1],
				       &mdata->dma_id[1]);
	if (ret) {
		dev_err(&pdev->dev, "missing/invalid capture DMA phandle (ret=%i)\n",
			ret);
		goto error;
	}

	/* Initialize our DAI data structure.  */
	mdata->dai[0].stream_name = "playback";
	mdata->dai[1].stream_name = "capture";
	mdata->dai[0].name = mdata->dai[0].stream_name;
	mdata->dai[1].name = mdata->dai[1].stream_name;

	mdata->card.probe = p1022_rdk_machine_probe;
	mdata->card.remove = p1022_rdk_machine_remove;
	mdata->card.name = pdev->name; /* The platform driver name */
	mdata->card.owner = THIS_MODULE;
	mdata->card.dev = &pdev->dev;
	mdata->card.num_links = 2;
	mdata->card.dai_link = mdata->dai;

	/* Register with ASoC */
	ret = snd_soc_register_card(&mdata->card);
	if (ret) {
		dev_err(&pdev->dev, "could not register card (ret=%i)\n", ret);
		goto error;
	}

	return 0;

error:
	kfree(mdata);
error_put:
	of_node_put(codec_np);
	return ret;
}

/**
 * p1022_rdk_remove - remove the platform device
 * @pdev: platform device pointer
 *
 * This function is called when the platform device is removed.
 */
static int p1022_rdk_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);
	struct machine_data *mdata =
		container_of(card, struct machine_data, card);

	snd_soc_unregister_card(card);
	kfree(mdata);

	return 0;
}

static struct platform_driver p1022_rdk_driver = {
	.probe = p1022_rdk_probe,
	.remove = p1022_rdk_remove,
	.driver = {
		/*
		 * The name must match 'compatible' property in the device tree,
		 * in lowercase letters.
		 */
		.name = "snd-soc-p1022rdk",
	},
};

/**
 * p1022_rdk_init - machine driver initialization.
 *
 * This function is called when this module is loaded.
 *
 * Returns: %0 on success or negative errno value on error
 */
static int __init p1022_rdk_init(void)
{
	struct device_node *guts_np;
	struct resource res;

	/* Get the physical address of the global utilities registers */
	guts_np = of_find_compatible_node(NULL, NULL, "fsl,p1022-guts");
	if (of_address_to_resource(guts_np, 0, &res)) {
		pr_err("snd-soc-p1022rdk: missing/invalid global utils node\n");
		of_node_put(guts_np);
		return -EINVAL;
	}
	guts_phys = res.start;
	of_node_put(guts_np);

	return platform_driver_register(&p1022_rdk_driver);
}

/**
 * p1022_rdk_exit - machine driver exit
 *
 * This function is called when this driver is unloaded.
 */
static void __exit p1022_rdk_exit(void)
{
	platform_driver_unregister(&p1022_rdk_driver);
}

late_initcall(p1022_rdk_init);
module_exit(p1022_rdk_exit);

MODULE_AUTHOR("Timur Tabi <timur@freescale.com>");
MODULE_DESCRIPTION("Freescale / iVeia P1022 RDK ALSA SoC machine driver");
MODULE_LICENSE("GPL v2");
