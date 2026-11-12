/* SPDX-License-Identifier: GPL-3.0-or-later */
/**
 * @file app_ao.c
 * @brief The app's Active Object: one state, and a worker for the work that waits.
 */
#include "app_ao.h"

#include "kode_worker.h"

#define TAG "app.ao"   /**< This file's log tag. */

#define APP_WORKER_NAME         "app_worker"   /**< The worker's task name. */
#define APP_WORKER_STACK_BYTES  8192U          /**< The worker's stack, in PSRAM: the least the SDK's own workers get. */
#define APP_WORKER_DEPTH        1U             /**< Jobs that may wait besides the one running. */

/** @brief The app's own signals: they start at MAX_SIG and are posted, never published. */
enum AppAoSignals
{
    APP_JOB_DONE_SIG = MAX_SIG   /**< The worker's job returned. */
};

/** @brief The Active Object's own state, private to this file. */
typedef struct {
    QActive       super;     /**< The QP/C base; must stay first. */
    kode_worker_t worker;    /**< Runs the work that waits, off the Active Object's step. */
    uint16_t      job_seq;   /**< The sequence number of the job the Active Object waits for. */
} AppAo;

static QEvtPtr     s_queue[KQP_AO_QUEUE_LEN];                            /**< Its event queue. */
static StackType_t s_stack[KQP_AO_STACK_BYTES / sizeof(StackType_t)];    /**< Its stack, in words. */
static AppAo       s_ao;                                                 /**< The one instance. */

QActive * const AO_App = &s_ao.super;

static kode_status_t AppAo_job(void *arg, uint32_t *value);
static QState AppAo_initial(AppAo * const me, void const * const par);
static QState AppAo_running(AppAo * const me, QEvt const * const e);

void AppAo_ctor(void)
{
    QActive_ctor(&s_ao.super, Q_STATE_CAST(&AppAo_initial));
}

void AppAo_start(void)
{
    /* A named task: an assert then names it instead of "AO". */
    QActive_setAttr(AO_App, TASK_NAME_ATTR, "app_ao");
    /* The lowest user priority, where it can only ever delay itself. */
    QACTIVE_START(AO_App, KODE_PRIO_USER_LOW, s_queue, Q_DIM(s_queue),
                  s_stack, sizeof(s_stack), (void *)0);
}

/**
 * @brief The job: the app's blocking work, run on the worker's task.
 * @param arg   What kode_worker_submit() was given; unused.
 * @param value Receives the one word the done event carries.
 * @return The status the done event carries.
 */
static kode_status_t AppAo_job(void *arg, uint32_t *value)
{
    (void)arg;
    /* The blocking kode_* calls go here; nothing that reaches the flash, kode_setting_* included. */
    *value = 0U;
    return KODE_OK;
}

/**
 * @brief The initial transition: subscribe to the system's signals, start the worker, then run.
 * @param me  The Active Object.
 * @param par Unused.
 * @return The transition to AppAo_running.
 */
static QState AppAo_initial(AppAo * const me, void const * const par)
{
    kode_status_t rc;

    (void)par;
    /* Subscribe here, then ask the owner for its state: what it announced at startup is past. */

    /* Here and not in a state: the initial transition runs once, and a worker starts once. */
    rc = kode_worker_start(&me->worker, &me->super, APP_WORKER_NAME, APP_WORKER_STACK_BYTES,
                           (uint8_t)KODE_PRIO_USER_LOW, (uint8_t)APP_WORKER_DEPTH);
    if ( KODE_OK != rc )
    {
        KLOG_E(TAG, "the worker did not start (status %d)", (int)rc);
    }
    else
    {
        /* The demonstration, to delete: one job out and its done event back. An app
         * submits from the handler of the event that starts the work. */
        rc = kode_worker_submit(&me->worker, &AppAo_job, NULL, (enum_t)APP_JOB_DONE_SIG,
                                &me->job_seq);
        if ( KODE_OK != rc )
        {
            KLOG_W(TAG, "the worker took no job (status %d)", (int)rc);
        }
    }
    KLOG_I(TAG, "up");
    return Q_TRAN(&AppAo_running);
}

/**
 * @brief The one state.
 * @param me The Active Object.
 * @param e  The event.
 * @return Q_HANDLED() for what it handles, Q_SUPER() otherwise.
 */
static QState AppAo_running(AppAo * const me, QEvt const * const e)
{
    QState status;

    switch ( e->sig )
    {
        case Q_ENTRY_SIG:
        {
            status = Q_HANDLED();
            break;
        }
        case APP_JOB_DONE_SIG:
        {
            KqpWorkerDoneEvt const * const done = Q_EVT_CAST(KqpWorkerDoneEvt);

            /* The worker decides nothing: an answer to an older job is not the one awaited. */
            if ( me->job_seq == done->seq )
            {
                KLOG_I(TAG, "job done: status %u, value %lu", (unsigned)done->status,
                       (unsigned long)done->value);
            }
            status = Q_HANDLED();
            break;
        }
        default:
        {
            status = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status;
}
