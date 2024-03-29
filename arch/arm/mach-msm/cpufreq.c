/* arch/arm/mach-msm/cpufreq.c
 *
 * MSM architecture cpufreq driver
 *
 * Copyright (C) 2007 Google, Inc.
 * Copyright (c) 2007 QUALCOMM Incorporated
 * Author: Mike A. Chan <mikechan@google.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 */

#include <linux/cpufreq.h>
#include <linux/earlysuspend.h>
#include <linux/init.h>
#include "acpuclock.h"

static int msm_cpufreq_target(struct cpufreq_policy *policy,
				unsigned int target_freq,
				unsigned int relation)
{
	int index;
	struct cpufreq_freqs freqs;
	struct cpufreq_frequency_table *table =
		cpufreq_frequency_get_table(policy->cpu);

	if (cpufreq_frequency_table_target(policy, table, target_freq, relation,
			&index)) {
		pr_err("cpufreq: invalid target_freq: %d\n", target_freq);
		return -EINVAL;
	}

	if (policy->cur == table[index].frequency)
		return 0;

	/*#ifdef CONFIG_AXI_SCREEN_POLICY
	// In the case of AXI, disable 128Mhz when the screen is on
	if ((cpufreq_get_screen_state()==1) && (table[index].frequency==128000)) {
	  return 0;
	}
	#endif // CONFIG_AXI_SCREEN_POLICY*/




#ifdef CONFIG_CPU_FREQ_DEBUG
	printk("msm_cpufreq_target %d r %d (%d-%d) selected %d\n", target_freq,
		relation, policy->min, policy->max, table[index].frequency);
#endif
	freqs.old = policy->cur;
	freqs.new = table[index].frequency;
	freqs.cpu = policy->cpu;
	cpufreq_notify_transition(&freqs, CPUFREQ_PRECHANGE);
	acpuclk_set_rate(table[index].frequency * 1000, 0);
	cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);
	return 0;
}

static int msm_cpufreq_verify(struct cpufreq_policy *policy)
{
	cpufreq_verify_within_limits(policy, policy->cpuinfo.min_freq,
			policy->cpuinfo.max_freq);
	return 0;
}

static int msm_cpufreq_init(struct cpufreq_policy *policy)
{
	struct cpufreq_frequency_table *table =
		cpufreq_frequency_get_table(policy->cpu);

	BUG_ON(cpufreq_frequency_table_cpuinfo(policy, table));
	policy->cur = acpuclk_get_rate();
#ifdef CONFIG_MSM_CPU_FREQ_SET_MIN_MAX
        policy->min = CONFIG_MSM_CPU_FREQ_MIN;
        policy->max = CONFIG_MSM_CPU_FREQ_MAX;
#endif
	policy->cpuinfo.transition_latency =
		acpuclk_get_switch_time() * NSEC_PER_USEC;
	//printk("msm_cpufreq_init(%d, %d\n", policy->min, policy->max);
	return 0;
}

static struct freq_attr *msm_cpufreq_attr[] = {
	&cpufreq_freq_attr_scaling_available_freqs,
	NULL,
};

static struct cpufreq_driver msm_cpufreq_driver = {
	/* lps calculations are handled here. */
	.flags		= CPUFREQ_STICKY | CPUFREQ_CONST_LOOPS,
	.init		= msm_cpufreq_init,
	.verify		= msm_cpufreq_verify,
	.target		= msm_cpufreq_target,
	.name		= "msm",
	.attr		= msm_cpufreq_attr,
};

static int __init msm_cpufreq_register(void)
{
  //printk("msm_cpufreq_register()\n");
	return cpufreq_register_driver(&msm_cpufreq_driver);
}

device_initcall(msm_cpufreq_register);
