/*
 * SPDX-License-Identifier: MPL-2.0
 * Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)
 */

package emu.skyline

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import com.google.android.material.bottomsheet.BottomSheetDialogFragment
import emu.skyline.loader.LoaderResult
import kotlinx.android.extensions.LayoutContainer
import kotlinx.android.synthetic.main.loader_error_dialog.*
import kotlinx.android.synthetic.main.loader_error_item.*

/**
 * This dialog shows the user the reason why ROMs weren't successfully read.
 */
class LoaderErrorDialog : BottomSheetDialogFragment() {
    companion object {
        fun newInstance(loaderErrors : HashMap<LoaderResult, ArrayList<String>>) : LoaderErrorDialog {
            val args = Bundle()
            args.putSerializable("loaderErrors", loaderErrors)

            val fragment = LoaderErrorDialog()
            fragment.arguments = args
            return fragment
        }
    }

    private lateinit var loaderErrors : HashMap<LoaderResult, ArrayList<String>>;
    private val errors by lazy { loaderErrors.keys.toTypedArray() }

    private class ErrorViewHolder(override val containerView : View) : RecyclerView.ViewHolder(containerView), LayoutContainer

    private inner class ErrorAdapter : RecyclerView.Adapter<ErrorViewHolder>() {
        override fun onCreateViewHolder(parent : ViewGroup, viewType : Int) = ErrorViewHolder(LayoutInflater.from(parent.context).inflate(R.layout.loader_error_item, parent, false))

        override fun getItemCount() = errors.size

        override fun onBindViewHolder(holder : ErrorViewHolder, position : Int) {
            holder.errorText.text = when (errors[position]) {
                LoaderResult.Success -> throw IllegalArgumentException("Success should not be part of errors")

                LoaderResult.ParsingError -> getString(R.string.invalid_file)

                LoaderResult.MissingTitleKey -> getString(R.string.missing_title_key)

                LoaderResult.MissingHeaderKey,
                LoaderResult.MissingTitleKek,
                LoaderResult.MissingKeyArea -> getString(R.string.incomplete_prod_keys)
            }

            holder.fileText.text = loaderErrors.getValue(errors[position]).joinToString("\n")
        }
    }

    override fun onCreateView(inflater : LayoutInflater, container : ViewGroup?, savedInstanceState : Bundle?) : View? {
        return inflater.inflate(R.layout.loader_error_dialog, container, false)
    }

    override fun onCreate(savedInstanceState : Bundle?) {
        super.onCreate(savedInstanceState)

        @Suppress("UNCHECKED_CAST")
        loaderErrors = arguments!!.getSerializable("loaderErrors") as HashMap<LoaderResult, ArrayList<String>>
    }

    override fun onViewCreated(view : View, savedInstanceState : Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        errorRecyclerView.setHasFixedSize(true)
        errorRecyclerView.adapter = ErrorAdapter()
    }
}