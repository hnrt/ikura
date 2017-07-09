// Copyright (C) 2014-2017 Hideaki Narita


#include <errno.h>
#include <glib.h>
#include <string.h>
#include "SigfpeHandler.h"


using namespace hnrt;


sigjmp_buf SigfpeHandler::env;
volatile int SigfpeHandler::code;


SigfpeHandler::SigfpeHandler()
{
    // First, block SIGFPE to prevent from being interrupted unexpectedly
    sigemptyset(&ssFpe);
    sigaddset(&ssFpe, SIGFPE);
    sigemptyset(&ssOld);
    int rc = pthread_sigmask(SIG_BLOCK, &ssFpe, &ssOld);
    if (rc)
    {
        g_printerr("Error: pthread_sigcmask failed: %s\n", strerror(rc));
    }

    // Then, install my handler for SIGFPE
    memset(&saFpe, 0, sizeof(saFpe));
    saFpe.sa_sigaction = handler;
    sigfillset(&saFpe.sa_mask);
    saFpe.sa_flags = SA_SIGINFO;
    memset(&saOld, 0, sizeof(saOld));
    if (sigaction(SIGFPE, &saFpe, &saOld))
    {
        g_printerr("Error: sigaction failed: %s\n", strerror(errno));
    }
}


SigfpeHandler::~SigfpeHandler()
{
    // First, restore the old handler setting for SIGFPE
    if (sigaction(SIGFPE, &saOld, NULL))
    {
        g_printerr("Error: sigaction failed: %s\n", strerror(errno));
    }

    // Then, restore the old signal mask setting for this thread
    int rc = pthread_sigmask(SIG_SETMASK, &ssOld, NULL);
    if (rc)
    {
        g_printerr("Error: pthread_sigcmask failed: %s\n", strerror(rc));
    }
}


void SigfpeHandler::resetCode()
{
    code = 0;

    // Unblock SIGFPE for the next sigsetjmp call
    int rc = pthread_sigmask(SIG_UNBLOCK, &ssFpe, NULL);
    if (rc)
    {
        g_printerr("Error: pthread_sigcmask failed: %s\n", strerror(rc));
    }
}


int SigfpeHandler::getCode()
{
    // Block SIGFPE again to prevent from being interrupted unexpectedly
    int rc = pthread_sigmask(SIG_BLOCK, &ssFpe, NULL);
    if (rc)
    {
        g_printerr("Error: pthread_sigcmask failed: %s\n", strerror(rc));
    }

    return code;
}


void SigfpeHandler::handler(int no, siginfo_t *si, void *uc)
{
    (void)uc; // unused

    if (no == SIGFPE)
    {
        code = si->si_code;
        siglongjmp(env, 1);
    }
}
